/*
 * This file is part of the cpyjava distribution.
 *   (https://github.com/m-g-90/cpyjava)
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
#include "pyjava/type.h"
#include "pyjava/conversion.h"
#include "pyjava/memory.h"
#include "pyjava/jvm.h"
#include "pyjava/config.h"

#ifdef __cplusplus
extern "C"{
#endif

static PyObject * pyjava_getType(PyObject *self, PyObject *args)
{
    (void)self;
    const char *name = NULL;
    PyObject * classloader = NULL;

    if (!PyArg_ParseTuple(args, "s|O", &name,&classloader)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"Illegal arguments");
        return NULL;
    }
	
    PYJAVA_START_JAVA(env);

    PyObject * ret = NULL;

    if (env){
        ret = (PyObject*) pyjava_classNameAsType(env,classloader,name);
    }
	
    PYJAVA_END_JAVA(env);

    if (!ret){
        if (!PyErr_Occurred()){
            PyErr_SetString(PyExc_Exception,"Class not found");
        }
    }

    return ret;
}


static PyObject * pyjava_callMethod(PyObject *self, PyObject *_args) {
    (void)self;

    PyObject * obj = NULL;
    const char * methname = NULL;
    PyObject * args = NULL;
    if (!PyArg_ParseTuple(_args, "OsO", &obj,&methname,&args)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.callMethod takes 3 arguments: object,method name,argument tuple");
        return NULL;
    }

    PYJAVA_START_JAVA(env);

    PyObject * ret = NULL;

    if (env){
        ret = pyjava_callFunction(env, obj,methname,args);
    }

    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"unknown error");
    }

    return ret;

}
static PyObject * pyjava_readField(PyObject *self, PyObject *_args) {
    (void)self;

    PyObject * obj = NULL;
    const char * methname = NULL;
    if (!PyArg_ParseTuple(_args, "Os", &obj,&methname)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.readField takes 2 arguments: object,field name");
        return NULL;
    }

    PYJAVA_START_JAVA(env);

    PyObject * ret = NULL;

    if (env){
        ret = pyjava_getField(env, obj,methname);
    }

    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"unknown error");
    }

    return ret;

}

static PyObject * pyjava_writeField(PyObject *self, PyObject *_args) {
    (void)self;

    PyObject * obj = NULL;
    PyObject * val = NULL;
    const char * methname = NULL;
    if (!PyArg_ParseTuple(_args, "OsO", &obj,&methname,&val)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.writeField takes 2 arguments: object,field name,value");
        return NULL;
    }

    PYJAVA_START_JAVA(env);

    if (env){
        pyjava_setField(env, obj,methname,val);
    }

    PYJAVA_END_JAVA(env);

    if (PyErr_Occurred()){
        return NULL;
    }

    Py_IncRef(val);
    return val;

}
static PyObject * pyjava_with_java_enter(PyObject * self, PyObject *_args){
    (void)self;
    (void)_args;

    pyjava_enter();
    Py_RETURN_TRUE;
}
static PyObject * pyjava_with_java_exit(PyObject * self, PyObject *_args){
    (void)self;
    (void)_args;

    pyjava_exit();
    Py_RETURN_TRUE;
}
static PyObject * pyjava_mem_stat(PyObject * self, PyObject *_args){
    (void)self;
    (void)_args;

    return pyjava_memory_statistics();
}
static PyObject * pyjava_symbols(PyObject * _dont_care, PyObject *_args){
    (void)_dont_care;

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

#ifdef __cplusplus
extern "C"
#endif
PYJAVA_DLLSPEC void JNICALL pyjava_registerObject(JNIEnv * env,jobject dont_care,jstring name,jobject object){
    (void)dont_care;
    if (name){
        const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,name, 0);
        if (tmp){
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();
            if (object){
                PyObject * val = pyjava_asPyObject(env,object);
                if (PyErr_Occurred()){
                    PyErr_Clear();
                    jclass excclass = PYJAVA_ENVCALL(env,FindClass,"java/lang/Exception");
                    PYJAVA_ENVCALL(env,ThrowNew,excclass,"Failed to convert java object to python object");
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
    cpyjavamethods,
    0,
    0,
    0,
    0
};

#ifdef __cplusplus
extern "C"
#endif
PYJAVA_DLLSPEC PyObject * PyInit_cpyjava(void) {
	
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
                    "        if item == 'type' or item == 'class':\n"
                    "            import cpyjava\n"
                    "            return cpyjava.getType(self.getName())\n"
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
                    "        import cpyjava\n"
                    "        return cpyjava.getType(self.getName())(*args)\n"
                    "\n"
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
