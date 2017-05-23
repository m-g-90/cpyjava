/*
 * This file is part of the cpyjava distribution.
 *   (https://bitbucket.org/j-f-v/cpyjava)
 * Copyright (c) 2017 Marc Greim.
 *
 * cpyjava is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * cpyjava is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pyjava/pyjava.h"
#include "pyjava/pyjava_jni.h"
#include "pyjava/type.h"
#include "pyjava/conversion.h"
#include "pyjava/memory.h"

#ifndef __cplusplus
#ifdef _MSC_VER
#define thread_local __declspec( thread )
#else
#define thread_local __thread
#endif
#endif

#ifdef PYJAVA_JVM_DLOPEN
        #include <dlfnc.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

static JavaVM * pyjava_jvmptr = NULL;

int pyjava_initJVM() {

    if (!pyjava_jvmptr){
        jint (*createJavaVM)(JavaVM **, void **, void *) = NULL;

#ifdef PYJAVA_JVM_FORCELINK
        if (!createJavaVM){
            createJavaVM = &JNI_CreateJavaVM;
        }
#endif
#ifdef PYJAVA_JVM_DLOPEN
        if (!createJavaVM){
            typedef jint JNICALL (*createJavaVM_t)(JavaVM **, void **, void *) ;
            createJavaVM = (createJavaVM_t) dlsym(NULL,"JNI_CreateJavaVM");
        }
#endif

        if (createJavaVM){
            JNIEnv *env;
            JavaVM *vm;
            JavaVMInitArgs initArgs;
            initArgs.version = JNI_VERSION_1_8;
            initArgs.ignoreUnrecognized = JNI_TRUE;
            JavaVMOption options[10];
            {
                options[0].extraInfo = NULL;
                options[0].optionString = "-Xcheck:jni";
            }
            int optioncount = 1;
            initArgs.options = options;
            initArgs.nOptions = optioncount;
            jint err = (*createJavaVM)(&vm, (void**)&env, &initArgs);
            if (err != JNI_OK){

            } else {
                PYJAVA_ENVCALL(vm,DetachCurrentThread);
                pyjava_jvmptr = vm;
                return 1;
            }
        }
    }

    return 0;
}

JavaVM * pyjava_getJVM(){

    // try getting existing jvms
    if (!pyjava_jvmptr){

        jint (*getCreatedJavaVMs)(JavaVM **, jsize, jsize *) = NULL;

#ifdef PYJAVA_JVM_FORCELINK
        if (!getCreatedJavaVMs){
            getCreatedJavaVMs = &JNI_GetCreatedJavaVMs;
        }
#endif
#ifdef PYJAVA_JVM_DLOPEN
        if (!getCreatedJavaVMs){
            typedef jint JNICALL (*getCreatedJavaVMs_t)(JavaVM **, jsize, jsize *);
            getCreatedJavaVMs = (getCreatedJavaVMs_t) dlsym(NULL,"JNI_GetCreatedJavaVMs");
        }
#endif

        if (getCreatedJavaVMs){
            JavaVM *vm;
            jsize vmcount;
            if (JNI_OK == getCreatedJavaVMs(&vm,1,&vmcount)){
                if (vmcount>0)
                    pyjava_jvmptr = vm;
            }
        }
    }

	return pyjava_jvmptr;
}
void pyjava_setJVM(JavaVM * jvm){
	pyjava_jvmptr = jvm;
}

static thread_local JNIEnv * _pyjava_enter_exit_env = NULL;
static thread_local int _pyjava_enter_exit_env_borrowed = 0;
static thread_local unsigned _pyjava_enter_exit_count = 0;
void pyjava_enter(){
    if (_pyjava_enter_exit_count==0){
        _pyjava_start_java(&_pyjava_enter_exit_env,&_pyjava_enter_exit_env_borrowed);
    }
    _pyjava_enter_exit_count++;
}

void pyjava_exit(){
    _pyjava_enter_exit_count--;
    if (_pyjava_enter_exit_count==0){
        _pyjava_end_java(&_pyjava_enter_exit_env,&_pyjava_enter_exit_env_borrowed);
    }
}

static thread_local JNIEnv * _pyjava_env = NULL;
void _pyjava_start_java(JNIEnv ** env, int * borrowed){
    if (_pyjava_env){
        *borrowed = 1;
        *env = _pyjava_env;
    } else {
        JavaVM * vm = pyjava_getJVM();
        if (PYJAVA_ENVCALL(vm,GetEnv,(void**)env,JNI_VERSION_1_8) == JNI_OK){
            _pyjava_env = *env;
            *borrowed = 2;
        } else {
            PYJAVA_ENVCALL(vm,AttachCurrentThread,(void **)env, NULL);
            _pyjava_env = *env;
            *borrowed = 0;
        }
    }
}

void _pyjava_end_java(JNIEnv ** env, int * borrowed){
    if (*borrowed){
        if (*borrowed == 2){
            *borrowed = 1;
            _pyjava_env = 0;
        }
    } else {
        JavaVM * vm = pyjava_getJVM();
        PYJAVA_ENVCALL(vm,DetachCurrentThread);
        _pyjava_env = NULL;
    }
}

static PyObject * pyjava_getType(PyObject *self, PyObject *args)
{
    const char *name = NULL;
    PyObject * classloader = NULL;

    if (!PyArg_ParseTuple(args, "s|O", &name,&classloader)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"Illegal arguments");
        return NULL;
    }
	
    PYJAVA_START_JAVA(env);

    PyObject * ret = (PyObject*) pyjava_classNameAsType(env,classloader,name);
	
    PYJAVA_END_JAVA(env);

    if (!ret){
        if (!PyErr_Occurred()){
            PyErr_SetString(PyExc_Exception,"Class not found");
        }
    }

    return ret;
}
static PyObject * _pyjava_class_getName(JNIEnv * env,jclass klass){
    jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
    jmethodID toString = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,toString);
    PYJAVA_IGNORE_EXCEPTION(env);
    const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
    PyObject * ret = PyUnicode_FromString(tmp);
    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
    PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
    return ret;
}
jclass _pyjava_getClass(JNIEnv * env, PyTypeObject * type);

static PyObject * pyjava_callMethod(PyObject *self, PyObject *_args) {

    PyObject * obj = NULL;
    const char * methname = NULL;
    PyObject * args = NULL;
    if (!PyArg_ParseTuple(_args, "OsO", &obj,&methname,&args)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.callMethod takes 3 arguments: object,method name,argument tuple");
        return NULL;
    }

    PYJAVA_START_JAVA(env);
    PyObject * ret = pyjava_callFunction(env, obj,methname,args);
    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"unknown error");
    }

    return ret;

}
static PyObject * pyjava_readField(PyObject *self, PyObject *_args) {

    PyObject * obj = NULL;
    const char * methname = NULL;
    if (!PyArg_ParseTuple(_args, "Os", &obj,&methname)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.readField takes 2 arguments: object,field name");
        return NULL;
    }

    PYJAVA_START_JAVA(env);
    PyObject * ret = pyjava_getField(env, obj,methname);
    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"unknown error");
    }

    return ret;

}

static PyObject * pyjava_writeField(PyObject *self, PyObject *_args) {

    PyObject * obj = NULL;
    PyObject * val = NULL;
    const char * methname = NULL;
    if (!PyArg_ParseTuple(_args, "OsO", &obj,&methname,&val)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.writeField takes 2 arguments: object,field name,value");
        return NULL;
    }

    PYJAVA_START_JAVA(env);
    pyjava_setField(env, obj,methname,val);
    PYJAVA_END_JAVA(env);

    if (PyErr_Occurred()){
        return NULL;
    }

    Py_IncRef(val);
    return val;

}
static PyObject * pyjava_with_java_enter(PyObject * self, PyObject *_args){
    pyjava_enter();
    Py_RETURN_TRUE;
}
static PyObject * pyjava_with_java_exit(PyObject * self, PyObject *_args){
    pyjava_exit();
    Py_RETURN_TRUE;
}
static PyObject * pyjava_mem_stat(PyObject * self, PyObject *_args){
    return pyjava_memory_statistics();
}
static PyObject * pyjava_symbols(PyObject * _dont_care, PyObject *_args){
    PyObject * self;
    if (!PyArg_ParseTuple(_args, "O", &self)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"");
        return NULL;
    }

    PyJavaType * type = NULL;
    if (pyjava_isJavaClass(self->ob_type)){
        type = (PyJavaType*)self->ob_type;
    }
    if (!type && PyType_CheckExact(self) && pyjava_isJavaClass((PyTypeObject*)self)){
        type = (PyJavaType*)self;
    }
    if (!type){
        PyErr_SetString(PyExc_Exception,"");
        return NULL;
    }
    PyObject * ret = type->dir;
    Py_IncRef(ret);
    return ret;

}



static PyObject * registeredObjects = NULL;
/**
 * @brief pyjava_getRegisteredObjects
 * @return BORROWED reference
 */
PyObject * pyjava_getRegisteredObjects(){
    if (!registeredObjects){
        registeredObjects = PyDict_New();
    }
    return registeredObjects;
}

JNIEXPORT void JNICALL pyjava_registerObject(JNIEnv * env,jstring name,jobject object){
    if (name){
        const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,name, 0);
        if (tmp){
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();
            if (object){
                PyObject * val = pyjava_asPyObject(env,object);
                if (PyErr_Occurred()){
                    PyErr_Clear();
                    PYJAVA_ENVCALL(env,ThrowNew,PYJAVA_ENVCALL(env,FindClass,"java/lang/Exception"),"Failed to convert java object to python object");
                } else {
                    PyDict_SetItemString(pyjava_getRegisteredObjects(),tmp,val);
                    Py_DecRef(val);
                }
            } else {
                PyDict_DelItemString(pyjava_getRegisteredObjects(),tmp);
            }
            PyGILState_Release(gstate);
            PYJAVA_ENVCALL(env,ReleaseStringUTFChars, name, tmp);
        }
    }
}

static PyMethodDef cpyjavamethods[] = {
    {"getType",  pyjava_getType, METH_VARARGS,""},
    {"callMethod",  pyjava_callMethod, METH_VARARGS,""},
    {"readField",  pyjava_readField, METH_VARARGS,""},
    {"writeField",  pyjava_writeField, METH_VARARGS,""},
    {"_with_java_enter",  pyjava_with_java_enter, METH_NOARGS,""},
    {"_with_java_exit",  pyjava_with_java_exit, METH_NOARGS,""},
    {"memstat",pyjava_mem_stat,METH_NOARGS,""},
    {"symbols",pyjava_symbols,METH_VARARGS,""},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cpyjavamodule = {
	PyModuleDef_HEAD_INIT,
    "cpyjava",
    "",
	-1,
	cpyjavamethods	
};

PyMODINIT_FUNC PyInit_cpyjava(void) {
	
    PyObject * ret = PyModule_Create(&cpyjavamodule);

    PyObject * ctx = PyDict_New();
    {
        {
            const char * classdef =
                    "class JavaPackage:\n"
                    "\n"
                    "    def __init__(self,parent,name):\n"
                    "        self.__dict__[\"_parent\"] = parent\n"
                    "        self.__dict__[\"_name\"] = name\n"
                    "\n"
                    "    def getName(self,append = None):\n"
                    "        if self._parent is not None:\n"
                    "            pname = self._parent.getName()\n"
                    "            if len(pname) > 0:\n"
                    "                ret = self._parent.getName() + '/' + self._name\n"
                    "            else:\n"
                    "                ret = self._name\n"
                    "        else:\n"
                    "            ret = self._name\n"
                    "        if append is not None:\n"
                    "            if len(ret) > 0:\n"
                    "                ret = ret + '/' + append\n"
                    "            else:\n"
                    "                ret = append\n"
                    "        return ret\n"
                    "\n"
                    "    def __getattr__(self, item):\n"
                    "        if item in self.__dict__:\n"
                    "            return self.__dict__[item]\n"
                    "        import cpyjava\n"
                    "        item = str(item)\n"
                    "        try:\n"
                    "            t = cpyjava.readField(cpyjava.getType(self.getName()),item)\n"
                    "            if t is not None:\n"
                    "                return t\n"
                    "        except Exception as ex:\n"
                    "            #print(self.getName(item))\n"
                    "            #print(repr(ex))\n"
                    "            pass\n"
                    "        return cpyjava.JavaPackage(self,item)\n"
                    "\n"
                    "    def __setattr__(self, item,value):\n"
                    "        if item in self.__dict__ or \"_name\" not in self.__dict__:\n"
                    "            self.__dict__[item] = value\n"
                    "        import cpyjava\n"
                    "        item = str(item)\n"
                    "        cpyjava.writeField(cpyjava.getType(self.getName()),item,value)\n"
                    "\n"
                    "    def __call__(self,*args):\n"
                    "        return cpyjava.getType(self.getName())(*args)"
                    ;


            PyObject* jpclass = PyRun_String(classdef, Py_single_input, PyEval_GetGlobals(), ctx);
            if (jpclass){
                Py_DecRef(jpclass);
            }
        }
        {
            const char * classdef =
                    "class env:\n"
                    "\n"
                    "    def __init__(self):\n"
                    "        pass\n"
                    "\n"
                    "    def __enter__(self):\n"
                    "        import cpyjava\n"
                    "        cpyjava._with_java_enter()\n"
                    "\n"
                    "    def __exit__(self):\n"
                    "        import cpyjava\n"
                    "        cpyjava._with_java_exit()\n"
                    "\n"
                    ;
            PyObject* jpclass = PyRun_String(classdef, Py_single_input, PyEval_GetGlobals(), ctx);
            if (jpclass){
                Py_DecRef(jpclass);
            }
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("JavaPackage", Py_eval_input, PyEval_GetGlobals(), ctx);
            if (packages){
                PyObject_SetAttrString(ret,"JavaPackage",packages);
            }
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("env()", Py_eval_input, PyEval_GetGlobals(), ctx);
            if (packages){
                PyObject_SetAttrString(ret,"env",packages);
            }
        }
        if (!PyErr_Occurred()){
            PyObject_SetAttrString(ret,"registeredObjects",pyjava_getRegisteredObjects());
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("JavaPackage(None,\"\")", Py_eval_input, PyEval_GetGlobals(), ctx);
            if (packages){
                PyObject_SetAttrString(ret,"packages",packages);
            }
        }
    }
	
    return ret;

}


#ifdef __cplusplus
}
#endif
