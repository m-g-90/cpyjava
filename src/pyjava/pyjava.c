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


static void _pyjava_handleDecoratorInheritance(PyJavaType * changed){
    PyObject * lcur = PyList_New(0);
    PyObject * lnext = PyList_New(0);

    PyList_Append(lcur,(PyObject*)changed);

    while (PyList_Size(lcur) > 0){
        //build lnext
        for (Py_ssize_t icur = 0;icur < PyList_GET_SIZE(lcur);icur++){
            PyTypeObject * tcur = (PyTypeObject*) PyList_GET_ITEM(lcur,icur);
            Py_ssize_t subpos = 0;
            PyObject * key,*value;
            if (tcur->tp_subclasses){
                while(PyDict_Next(tcur->tp_subclasses,&subpos,&key,&value)){
                    if (value && PyWeakref_Check(value)){
                        PyObject * subtype = PyWeakref_GetObject(value);
                        if (subtype){
                            if (PyType_CheckExact(subtype) && pyjava_isJavaClass((PyTypeObject*)subtype)){
                                PyList_Append(lnext,subtype);
                            }
                            Py_DecRef(subtype);
                        }
                    }
                }
            }
        }

        // update inheritance for lnext list
        for (Py_ssize_t inext = 0;inext < PyList_GET_SIZE(lnext);inext++){
            PyTypeObject * tnext = (PyTypeObject*) PyList_GET_ITEM(lnext,inext);
            pyjava_inherit_decorators(tnext);
        }

        //prepare next iteration
        Py_DecRef(lcur);
        lcur = lnext;
        lnext = PyList_New(0);

    }

    Py_DecRef(lcur);
    Py_DecRef(lnext);


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

    if (jtype->decoration.tp_call){
        Py_DecRef(jtype->decoration.tp_call);
        jtype->decoration.tp_call = NULL;
        jtype->decoration.inherited.tp_call = 1;
    }

    if (callback != Py_None){
        Py_IncRef(callback);
        jtype->decoration.tp_call = callback;
        jtype->decoration.inherited.tp_call = 0;
    }

    _pyjava_handleDecoratorInheritance(jtype);

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

    if (jtype->decoration.tp_getattro){
        Py_DecRef(jtype->decoration.tp_getattro);
        jtype->decoration.tp_getattro = NULL;
        jtype->decoration.inherited.tp_getattro = 1;
    }

    if (callback != Py_None){
        Py_IncRef(callback);
        jtype->decoration.tp_getattro = callback;
        jtype->decoration.inherited.tp_getattro = 0;
    }

    _pyjava_handleDecoratorInheritance(jtype);

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

    if (jtype->decoration.tp_setattro){
        Py_DecRef(jtype->decoration.tp_setattro);
        jtype->decoration.tp_setattro = NULL;
        jtype->decoration.inherited.tp_setattro = 1;
    }

    if (callback != Py_None){
        Py_IncRef(callback);
        jtype->decoration.tp_setattro = callback;
        jtype->decoration.inherited.tp_setattro = 0;
    }

    _pyjava_handleDecoratorInheritance(jtype);

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

static PyObject * pyjava_typeAsClassObject(PyObject * module, PyObject *_args){

    (void)module;

    PyObject * self = NULL;
    if (!PyArg_ParseTuple(_args, "O", &self)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"");
        return NULL;
    }

    if (!PyType_CheckExact(self) || !pyjava_isJavaClass((PyTypeObject*)self)){
        PyErr_SetString(PyExc_Exception,"Not a java type.");
    }

    PyObject * ret = NULL;

    PYJAVA_START_JAVA(env);
    if (env){
        ret = pyjava_asWrappedObject(env,self);
    }
    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"conversion failed");
    }

    return ret;

}

static PyObject * pyjava_cast(PyObject * module, PyObject *_args){

    (void)module;

    PyObject * type = NULL;
    PyObject * obj = NULL;
    if (!PyArg_ParseTuple(_args, "OO", &type,&obj)){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_Exception,"2 arguments expected: type,object");
        return NULL;
    }

    if (!PyType_CheckExact(type) || !pyjava_isJavaClass((PyTypeObject*)type)){
        PyErr_SetString(PyExc_Exception,"argument 1 is not a java type");
    }

    PyObject * ret = NULL;

    PYJAVA_START_JAVA(env);
    if (env){
        jvalue jval;
        jval.l = NULL;
        const char ntype = pyjava_getNType(env,((PyJavaType*)type)->klass);
        if (ntype=='L' || ntype=='['){
            if (pyjava_asJObject(env,obj,((PyJavaType*)type)->klass,ntype,&jval)){
                ret = pyjava_asUnconvertedWrappedObject(env,jval.l);
            }
        } else {
            PyErr_SetString(PyExc_Exception,"explicit conversion to primitive type is not supported");
        }
    }
    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"conversion failed");
    }

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
PYJAVA_DLLSPEC void pyjava_registerObject(JNIEnv * env,jobject dont_care,jstring name,jobject object){
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
    {"getType",  pyjava_getType, METH_VARARGS,"getType(className): get a type object for the named java class. e.g. getType('java/lang/String')"},
    {"callMethod",  pyjava_callMethod, METH_VARARGS,"callMethod(obj,methodName,*args): call a java member method"},
    {"hasMethod",  pyjava_hasMethod, METH_VARARGS,"hasMethod(obj,methodName): returns true if a java member method with that name exists"},
    {"readField",  pyjava_readField, METH_VARARGS,"readField(obj,fieldName): get the value of a java field."},
    {"writeField",  pyjava_writeField, METH_VARARGS,"writeField(obj,fieldName,value): set the value of a java field."},
    {"_with_java_enter",  pyjava_with_java_enter, METH_NOARGS,"internal function"},
    {"_with_java_exit",  pyjava_with_java_exit, METH_NOARGS,"internal function"},
    {"setCallDecorator",pyjava_setCallDecorator,METH_VARARGS,"setCallDecorator(javaType,callback): adds a decorator callback that will be called instead. callback should expect the arguments (self,decoself,*args,**kwargs). decoself is a special object that points to the same java object as the decorated self object, but decorators are disabled if decoself is used."},
    {"setGetterDecorator",pyjava_setGetterDecorator,METH_VARARGS,"setGetterDecorator(javaType,callback): adds a decorator callback that will be called instead. callback should expect the arguments (self,decoself,key). decoself is a special object that points to the same java object as the decorated self object, but decorators are disabled if decoself is used."},
    {"setSetterDecorator",pyjava_setSetterDecorator,METH_VARARGS,"setSetterDecorator(javaType,callback): adds a decorator callback that will be called instead. callback should expect the arguments (self,decoself,key,value) AND (self,decoself,key). The later one is nedded to support 'del'. decoself is a special object that points to the same java object as the decorated self object, but decorators are disabled if decoself is used."},
    {"selftest",  pyjava_selftest, METH_NOARGS,"selftest(): runs the builtin selftest code snippets"},
    {"memstat",pyjava_mem_stat,METH_VARARGS,"memstat(): returns a string with information about the memory consumption of cpyjava."},
    {"symbols",pyjava_symbols,METH_VARARGS,"symbols(jobject): returns a tuple of symbols of the given object. Behaves like dir()."},
    {"typeAsClassObject",pyjava_typeAsClassObject,METH_VARARGS,"typeAsClassObject(javaType): returns a wrapped java.lang.Class object from a java type (e.g. optained by cpyjava.getType('java/lang/String'))"},
    {"cast",pyjava_cast,METH_VARARGS,"case(jtype,any_object),returns a wrapped java object of the specified java type."},
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
                    "        self._import('cpyjava')._with_java_exit(dc0,dc1,dc2)\n"
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
