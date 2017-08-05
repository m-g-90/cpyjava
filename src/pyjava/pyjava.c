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
#include "pyjava/selftest.h"

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

static PyObject * pyjava_hasMethod(PyObject *self, PyObject *_args) {
    (void)self;

    PyObject * obj = NULL;
    const char * methname = NULL;
    PyObject * args = NULL;
    if (!PyArg_ParseTuple(_args, "Os", &obj,&methname,&args)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.callMethod takes 2 arguments: object,method name");
        return NULL;
    }

    PYJAVA_START_JAVA(env);

    int ret = 0;

    if (env){
        ret = pyjava_hasFunction(env, obj,methname);
    }

    PYJAVA_END_JAVA(env);

    if (PyErr_Occurred()){
        return NULL;
    }

    if (ret){
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }

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

static PyObject * pyjava_selftest(PyObject *  self,PyObject * _args){
    (void)self;
    (void)_args;

    const char ** test = pyjava_selftests;
    int failed = 0;
    while (*test){
        PyRun_SimpleString(*test);
        if (PyErr_Occurred()){
            PyErr_Print();
            PyErr_Clear();
            failed = 1;
        }
        test++;
    }

    if (failed){
        PyErr_SetString(PyExc_RuntimeError,"cpyjava selftest failed");
        return NULL;
    }

    Py_RETURN_NONE;

}

static PyObject * pyjava_setCallDecorator(PyObject * self, PyObject *_args){
    (void)self;

    PyObject * type = NULL;
    PyObject * callback = NULL;
    if (!PyArg_ParseTuple(_args, "OO", &type,&callback)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.setCallDecorator takes 2 arguments. A java type and a callback function");
        return NULL;
    }

    if (PyType_CheckExact(type) && !pyjava_isJavaClass((PyTypeObject*)type)){
        PyErr_SetString(PyExc_Exception,"cpyjava.setCallDecorator takes a java type object as the first argument.");
        return NULL;
    }
    PyJavaType * jtype = (PyJavaType*) type;

    Py_IncRef(callback);
    jtype->decoration.tp_call = callback;

    Py_RETURN_NONE;

}

static PyObject * pyjava_setGetterDecorator(PyObject * self, PyObject *_args){
    (void)self;

    PyObject * type = NULL;
    PyObject * callback = NULL;
    if (!PyArg_ParseTuple(_args, "OO", &type,&callback)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.setGetterDecorator takes 2 arguments. A java type and a callback function");
        return NULL;
    }

    if (PyType_CheckExact(type) && !pyjava_isJavaClass((PyTypeObject*)type)){
        PyErr_SetString(PyExc_Exception,"cpyjava.setGetterDecorator takes a java type object as the first argument.");
        return NULL;
    }
    PyJavaType * jtype = (PyJavaType*) type;

    Py_IncRef(callback);
    jtype->decoration.tp_getattro = callback;

    Py_RETURN_NONE;

}

static PyObject * pyjava_setSetterDecorator(PyObject * self, PyObject *_args){
    (void)self;

    PyObject * type = NULL;
    PyObject * callback = NULL;
    if (!PyArg_ParseTuple(_args, "OO", &type,&callback)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.setSetterDecorator takes 2 arguments. A java type and a callback function");
        return NULL;
    }

    if (PyType_CheckExact(type) && !pyjava_isJavaClass((PyTypeObject*)type)){
        PyErr_SetString(PyExc_Exception,"cpyjava.setSetterDecorator takes a java type object as the first argument.");
        return NULL;
    }
    PyJavaType * jtype = (PyJavaType*) type;

    Py_IncRef(callback);
    jtype->decoration.tp_setattro = callback;

    Py_RETURN_NONE;

}

static PyObject * pyjava_mem_stat(PyObject * self, PyObject *_args){
    (void)self;

    const char * cmd = NULL;
    if (!PyArg_ParseTuple(_args, "|s", &cmd)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"cpyjava.memstat takes 1 string arguments");
        return NULL;
    }

    return pyjava_memory_statistics(cmd);
}
static PyObject * pyjava_symbols(PyObject * module, PyObject *_args){

    (void)module;

    PyObject * self = NULL;
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
    {"hasMethod",  pyjava_hasMethod, METH_VARARGS,""},
    {"readField",  pyjava_readField, METH_VARARGS,""},
    {"writeField",  pyjava_writeField, METH_VARARGS,""},
    {"_with_java_enter",  pyjava_with_java_enter, METH_NOARGS,""},
    {"_with_java_exit",  pyjava_with_java_exit, METH_NOARGS,""},
    {"setCallDecorator",pyjava_setCallDecorator,METH_VARARGS,""},
    {"setGetterDecorator",pyjava_setGetterDecorator,METH_VARARGS,""},
    {"setSetterDecorator",pyjava_setSetterDecorator,METH_VARARGS,""},
    {"selftest",  pyjava_selftest, METH_NOARGS,""},
    {"memstat",pyjava_mem_stat,METH_VARARGS,""},
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

static PyObject * _pyjava_module = NULL;

#ifdef __cplusplus
extern "C"
#endif
PYJAVA_DLLSPEC PyObject * PyInit_cpyjava(void) {
	
    PyObject * ret = PyModule_Create(&cpyjavamodule);

    _pyjava_module = ret;

    PyObject * globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    PyObject * ctx = PyDict_Copy(globals);
    {
        {
            const char * classdef =
                    "class JavaPackage:\n"
                    "\n"
                    "    _import = __builtins__['__import__']\n"
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
                    "        cpyjava = self._import('cpyjava')\n"
                    "        item = str(item)\n"
                    "        try:\n"
                    "            if cpyjava.hasMethod(cpyjava.getType(self.getName()),item):\n"
                    "                return (lambda c,t,i: lambda *args : c.callMethod(t,i,args))(cpyjava,cpyjava.getType(self.getName()),item)\n"
                    "        except:\n"
                    "            pass\n"
                    "        try:\n"
                    "            t = cpyjava.readField(cpyjava.getType(self.getName()),item)\n"
                    "            if t is not None:\n"
                    "                return t\n"
                    "        except Exception as ex:\n"
                    "            #print(self.getName(item))\n"
                    "            #print(repr(ex))\n"
                    "            pass\n"
                    "        if item == 'type' or item == 'class':\n"
                    "            return cpyjava.getType(self.getName())\n"
                    "        return cpyjava.JavaPackage(self,item)\n"
                    "\n"
                    "    def __setattr__(self, item,value):\n"
                    "        if item in self.__dict__ or \"_name\" not in self.__dict__:\n"
                    "            self.__dict__[item] = value\n"
                    "        cpyjava = self._import('cpyjava')\n"
                    "        item = str(item)\n"
                    "        cpyjava.writeField(cpyjava.getType(self.getName()),item,value)\n"
                    "\n"
                    "    def __call__(self,*args):\n"
                    "        cpyjava = self._import('cpyjava')\n"
                    "        return cpyjava.getType(self.getName())(*args)\n"
                    "\n"
                    ;


            PyObject* jpclass = PyRun_String(classdef, Py_single_input, globals, ctx);
            if (jpclass){
                Py_DecRef(jpclass);
            } else {
                PyErr_Print();
            }
        }
        {
            const char * classdef =
                    "class env:\n"
                    "\n"
                    "    _import = __builtins__['__import__']\n"
                    "\n"
                    "    def __init__(self):\n"
                    "        pass\n"
                    "\n"
                    "    def __enter__(self):\n"
                    "        self._import('cpyjava')._with_java_enter()\n"
                    "\n"
                    "    def __exit__(self):\n"
                    "        self._import('cpyjava')._with_java_exit()\n"
                    "\n"
                    ;
            PyObject* jpclass = PyRun_String(classdef, Py_single_input, globals, ctx);
            if (jpclass){
                Py_DecRef(jpclass);
            }
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("JavaPackage", Py_eval_input, globals, ctx);
            if (packages){
                PyObject_SetAttrString(ret,"JavaPackage",packages);
            }
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("env()", Py_eval_input, globals, ctx);
            if (packages){
                PyObject_SetAttrString(ret,"env",packages);
            }
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("env._import", Py_eval_input, globals, ctx);
            if (packages){
                PyObject_SetAttrString(ret,"_import",packages);
            }
        }
        if (!PyErr_Occurred()){
            PyObject_SetAttrString(ret,"registeredObjects",pyjava_getRegisteredObjects());
        }
        if (!PyErr_Occurred()){
            PyObject * packages = PyRun_String("JavaPackage(None,\"\")", Py_eval_input, globals, ctx);
            if (packages){
                PyObject_SetAttrString(ret,"packages",packages);
            }
        }
    }
	
    Py_DecRef(globals);
    Py_DecRef(ctx);

    return ret;

}

PYJAVA_DLLSPEC PyObject * pyjava_getModule(void) {
    if (!_pyjava_module){
        PyObject * m = PyImport_ImportModule("cpyjava");
        if (m){
            Py_DecRef(m);
        }
    }

    if (_pyjava_module){
        Py_IncRef(_pyjava_module);
        return _pyjava_module;
    }

    if (!PyErr_Occurred())
        PyErr_BadInternalCall();
    return NULL;
}

#ifdef __cplusplus
}
#endif
