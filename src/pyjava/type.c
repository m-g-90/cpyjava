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

#include "pyjava/type.h"
#include "pyjava/conversion.h"
#include "pyjava/memory.h"
#include "pyjava/method_cache.h"
#include "pyjava/type_cache.h"
#include "pyjava/jvm.h"
#include "pyjava/type_helpers.h"
#include "pyjava/type_extensions.h"

#ifdef __cplusplus
extern "C"{
#endif

void _pyjava_removededupobject(JNIEnv * env,PyJavaObject * o);
void pyjava_type_dealloc(PyJavaObject * self)
{

    PYJAVA_START_JAVA(env);
    _pyjava_removededupobject(env,self);
    jobject obj = self->obj;
    self->obj = NULL;
    pyjava_free(self);
    PYJAVA_ENVCALL(env,DeleteGlobalRef,obj);
    PYJAVA_END_JAVA(env);
}

static int pyjava_cstring_hash(const char * c){
    int ret = 0;
    while (*c){
        ret = ret*23 + (+*c)*3;
        c++;
    }
    return ret;
}

Py_hash_t pyjava_type_hash(PyJavaObject * self){
    if (!self->obj){
        PyErr_SetString(PyExc_RuntimeWarning,"Tried to access a deleted java object");
        return 0;
    }
    PYJAVA_START_JAVA(env);
    jobject obj = PYJAVA_ENVCALL(env,NewLocalRef,self->obj);
    jmethodID hash = ((PyJavaType*)Py_TYPE(self))->hashCode;
    PYJAVA_YIELD_GIL(gstate);
    jint ret = PYJAVA_ENVCALL(env,CallIntMethod,obj,hash);
    PYJAVA_ENVCALL(env,DeleteLocalRef,obj);
    PYJAVA_RESTORE_GIL(gstate);
    PYJAVA_END_JAVA(env);
    return (Py_hash_t) ret;
}

PyObject* pyjava_type_repr(PyJavaObject * self){
    if (!self->obj){
        PyErr_SetString(PyExc_RuntimeWarning,"Tried to access a deleted java object");
        return 0;
    }
    PYJAVA_START_JAVA(env);
    jobject obj = PYJAVA_ENVCALL(env,NewLocalRef,self->obj);
    jmethodID toString = ((PyJavaType*)Py_TYPE(self))->toString;
    PYJAVA_YIELD_GIL(gstate);
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,obj,toString);
    PYJAVA_RESTORE_GIL(gstate);
    PyObject * ret = NULL;
    if (!pyjava_exception_java2python(env)){
        const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
        ret = PyUnicode_FromString(tmp);
        PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    }
    PYJAVA_ENVCALL(env,DeleteLocalRef,obj);
    PYJAVA_END_JAVA(env);
    return ret;
}

static PyObject * pyjava_type_call(PyJavaObject *self, PyObject *args, PyObject *kw){



    PyJavaType * type = (PyJavaType*) self->ob_base.ob_type;
    if (type->decoration.tp_call && !self->flags.entries.decoself) {
        PYJAVA_START_JAVA(env);
        Py_ssize_t argcount = args ? PyTuple_Size(args) : 0;
        PyObject * decoargs = PyTuple_New(argcount+2);
        Py_IncRef((PyObject*)self);
        PyTuple_SetItem(decoargs,0,(PyObject*)self);
        PyTuple_SetItem(decoargs,1,(PyObject*)pyjava_asUndecoratedObject(env,(PyObject*)self));
        for (Py_ssize_t i = 0;i < argcount;i++){
            PyObject * cur = PyTuple_GetItem(args,i);
            Py_IncRef(cur);
            PyTuple_SetItem(decoargs,i+2,cur);
        }
        PyObject * decoret = PyObject_Call(type->decoration.tp_call,decoargs,kw);
        Py_DecRef(decoargs);
        PYJAVA_END_JAVA(env);
        return decoret;
    }

    if (type->tp_call_impl){
        return type->tp_call_impl((PyObject*)self,args,kw);
    }

    PyErr_SetString(PyExc_TypeError,"Not callable.");
    return NULL;

}

static PyObject * _pyjava_method_helper = NULL;
static PyObject* pyjava_type_getattro( PyObject* self, PyObject* pyname){

    if (((PyJavaType*)self->ob_type)->decoration.tp_getattro && !((PyJavaObject*)self)->flags.entries.decoself){
        PYJAVA_START_JAVA(env);
        PyObject * decoargs = PyTuple_New(3);
        Py_IncRef((PyObject*)self);
        PyTuple_SetItem(decoargs,0,(PyObject*)self);
        PyTuple_SetItem(decoargs,1,(PyObject*)pyjava_asUndecoratedObject(env,(PyObject*)self));
        Py_IncRef(pyname);
        PyTuple_SetItem(decoargs,2,pyname);
        PyObject * decoret = PyObject_Call(((PyJavaType*)self->ob_type)->decoration.tp_getattro,decoargs,NULL);
        Py_DecRef(decoargs);
        PYJAVA_END_JAVA(env);
        return decoret;
    }

    if (!PyUnicode_CheckExact(pyname)){
        PyErr_SetString(PyExc_AttributeError,"java object only has string attributes");
        return NULL;
    }

    int method = 0;

    const char  *name = PyUnicode_AsUTF8(pyname);
    if (!strcmp(name,"__dict__")){
        if (pyjava_isJavaClass(self->ob_type)){
            PyObject * ret = PyDict_New();
            for (Py_ssize_t i = 0;i<PyTuple_Size(((PyJavaType*)self->ob_type)->dir);i++){
                PyDict_SetItem(ret,PyTuple_GET_ITEM(((PyJavaType*)self->ob_type)->dir,i),Py_None);
            }
            return ret;
        }
    }
    PYJAVA_START_JAVA(env);
    const int namehash = pyjava_cstring_hash(name);
    if (pyjava_isJavaClass(self->ob_type)){
        PyJavaMethod * _meth = ((PyJavaType*)self->ob_type)->methods[(unsigned)namehash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
        while (_meth){
            PyJavaMethod * meth = _meth;
            _meth = _meth->next;
            if (strcmp(meth->name,name)){
                continue;
            }
            method = 1;
            break;
        }
    }
    PyObject * ret = NULL;
    if (method){
        if (!_pyjava_method_helper){
            PyObject * globals = PyDict_New();
            PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
            if (globals){
                _pyjava_method_helper = PyRun_String("(lambda c: lambda s,n: lambda *args,**kvargs: c.callMethod(s,n,args))(__builtins__['__import__']('cpyjava'))", Py_eval_input, globals, globals);
                Py_DecRef(globals);
            }
        }
        if (_pyjava_method_helper){
            PyObject * args = PyTuple_New(2);
            Py_IncRef(self);
            Py_IncRef(pyname);
            PyTuple_SetItem(args,0,self);
            PyTuple_SetItem(args,1,pyname);
            ret = PyObject_Call(_pyjava_method_helper,args,NULL);
        } else {
            ret = NULL;
           PyErr_BadInternalCall();
        }
    } else {
        ret = pyjava_getField(env,self,name);
    }
    PYJAVA_END_JAVA(env);
    return ret;
}
static int pyjava_type_setattro( PyObject* self, PyObject* pyname,PyObject * value){

    if (((PyJavaType*)self->ob_type)->decoration.tp_setattro && !((PyJavaObject*)self)->flags.entries.decoself){
        PYJAVA_START_JAVA(env);
        PyObject * decoargs = PyTuple_New(value ? 4 : 3);
        Py_IncRef((PyObject*)self);
        PyTuple_SetItem(decoargs,0,(PyObject*)self);
        PyTuple_SetItem(decoargs,1,(PyObject*)pyjava_asUndecoratedObject(env,(PyObject*)self));
        Py_IncRef(pyname);
        PyTuple_SetItem(decoargs,2,pyname);
        if (value){
            Py_IncRef(value);
            PyTuple_SetItem(decoargs,3,value);
        }
        PyObject * decoret = PyObject_Call(((PyJavaType*)self->ob_type)->decoration.tp_setattro,decoargs,NULL);
        Py_DecRef(decoargs);
        PYJAVA_END_JAVA(env);
        if (decoret)
            Py_DecRef(decoret);
        return PyErr_Occurred() ? -1 : 0;
    }

    if (!PyUnicode_CheckExact(pyname)){
        PyErr_SetString(PyExc_AttributeError,"java object only has string attributes");
        return 0;
    }
    PYJAVA_START_JAVA(env);
    const char  *name = PyUnicode_AsUTF8(pyname);
    pyjava_setField(env,self,name,value);
    PYJAVA_END_JAVA(env);
    if (PyErr_Occurred()){
        return -1;
    }
    return 0;
}

static PyObject * pyjava_type_richcompare(PyObject * a, PyObject *b, int op){
    if (a == b){
        Py_RETURN_TRUE;
    }
    if ((op == Py_EQ || op == Py_NE) && pyjava_isJavaClass(b->ob_type)){
        Py_IncRef(b);
        PYJAVA_START_JAVA(env);
        if (!pyjava_isJavaClass(b->ob_type)){
            jvalue jval;
            if (pyjava_asJObject(env,b,pyjava_object_class(env),'L',&jval)){
                Py_DecRef(b);
                b = pyjava_asUnconvertedWrappedObject(env,jval.l);
                PYJAVA_ENVCALL(env,DeleteLocalRef,jval.l);
            }
        }
        if (pyjava_isJavaClass(b->ob_type)){
            int result = pyjava_object_equal(env,((PyJavaObject*)a)->obj,((PyJavaObject*)b)->obj);
            int erroccured = pyjava_exception_java2python(env);
            PYJAVA_END_JAVA(env);
            if (!erroccured){
                if (result<0){
                    Py_DecRef(b);
                    Py_RETURN_NOTIMPLEMENTED;
                }
                if (op == Py_EQ){
                    if (result){
                        Py_DecRef(b);
                        Py_RETURN_TRUE;
                    } else {
                        Py_DecRef(b);
                        Py_RETURN_FALSE;
                    }
                } else {
                    if (result){
                        Py_DecRef(b);
                        Py_RETURN_FALSE;
                    } else {
                        Py_DecRef(b);
                        Py_RETURN_TRUE;
                    }
                }
            } else {
                PyErr_Clear();
            }
        } else {
            PYJAVA_END_JAVA(env);
        }
        Py_DecRef(b);
    }

    PyObject * ret = NULL;

    if (((PyJavaType*)a->ob_type)->tp_richcompare_impl){
        ret = ((PyJavaType*)a->ob_type)->tp_richcompare_impl(a,b,op);
    }

    if (PyErr_Occurred()){
        PyErr_Clear();
    } else {
        if (ret)
            return ret;
    }

    Py_RETURN_NOTIMPLEMENTED;
}

char pyjava_getNType(JNIEnv * env,jclass klass){
    if (!klass){
        return (char)0;
    }
    char ret = 0;
    jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
    jmethodID toString = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getCanonicalName","()Ljava/lang/String;");
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,toString);
    PYJAVA_IGNORE_EXCEPTION(env);
    const char  *tmp = "L";
    if (jname)
        tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
    if (!strcmp("void",tmp)){
        ret = 'V';
    } else if (!strcmp("boolean",tmp)){
        ret = 'Z';
    } else if (!strcmp("byte",tmp)){
        ret = 'B';
    } else if (!strcmp("char",tmp)){
        ret = 'C';
    } else if (!strcmp("short",tmp)){
        ret = 'S';
    } else if (!strcmp("int",tmp)){
        ret = 'I';
    } else if (!strcmp("long",tmp)){
        ret = 'J';
    } else if (!strcmp("float",tmp)){
        ret = 'F';
    } else if (!strcmp("double",tmp)){
        ret = 'D';
    } else if (tmp[0]=='['){
        ret = '[';
    } else {
        ret = 'L';
    }
    if (jname){
        PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    }
    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
    return ret;
}

static int pyjava_type_init(PyJavaObject *self, PyObject *args, PyObject *kwds)
{
    (void)self;
    (void)args;
    (void)kwds;

    return 0;
}

PyObject *pyjava_type_alloc(PyTypeObject *self, Py_ssize_t nitems){
    (void)self;
    (void)nitems;

    PyErr_SetString(PyExc_Exception,"don't use tp_alloc of pyjava objects");
    return NULL;
}

PyObject * pyjava_type_new(PyJavaType * type, PyObject *args, PyObject *kwds){

    if (Py_None == args){
        args = NULL;
    }
    if (Py_None == kwds){
        args = NULL;
    }

    if (args && !PyTuple_CheckExact(args)){
        PyErr_SetString(PyExc_Exception,"");
        return NULL;
    }
    if (kwds && !PyDict_CheckExact(kwds)){
        PyErr_SetString(PyExc_Exception,"");
        return NULL;
    }

    Py_ssize_t kwarglen = 0;
    if (kwds) {
        kwarglen = PyDict_Size(kwds);
    }

    // TODO?: handle kwargs
    if (kwarglen > 0){
        PyErr_SetString(PyExc_Exception,"pyjava doesn't support keyword arguments");
        return NULL;
    }

    PyObject * self = NULL;
    PYJAVA_START_JAVA(env);
    self = pyjava_callFunction(env,(PyObject *)type,"<init>",args);
    if (pyjava_exception_java2python(env)) {
        if (self){
            Py_DecRef(self);
            self = NULL;
        }
    }
    PYJAVA_END_JAVA(env);

    return self;

}

int pyjava_isJavaClass(PyTypeObject * type){
    if (type){
        return (void*)type->tp_new == (void*)pyjava_type_new;
    } else {
        return 0;
    }
}

PyObject * pyjava_callFunction(JNIEnv * env, PyObject * _obj,const char * name,PyObject * tuple){
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            type=(PyJavaType*)_obj->ob_type;
            obj=(PyJavaObject*)_obj;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    }
    if (tuple && !PyTuple_CheckExact(tuple)){
        PyErr_BadArgument();
        return NULL;
    }

    if (!name){
        PyErr_BadArgument();
        return NULL;
    }

    const int namehash = pyjava_cstring_hash(name);

    Py_ssize_t argcount = tuple ? PyTuple_Size(tuple) : 0;

    PyJavaMethod * _method = type->methods[(unsigned)namehash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
    while (_method){
        PyJavaMethod * method = _method;
        _method = _method->next;
        if (strcmp(method->name,name)){
            continue;
        }
        if ((int)argcount != method->parametercount){
            continue;
        }
        if (!method->modifiers.isStatic && !obj){
            continue;
        }

        jvalue * jargs = (jvalue*)pyjava_malloc(sizeof(jvalue)*argcount);
        int err = 0;
        for (Py_ssize_t argi = 0;argi<argcount;argi++){
            if (!pyjava_asJObject(env,PyTuple_GET_ITEM(tuple,argi),method->parameters[argi].klass,method->parameters[argi].ntype,&jargs[argi])){
                err = 1;
                if (PyErr_Occurred()){
                    PyErr_Clear();
                }
                for (argi=argi-1;argi>=0;argi--){
                    if (method->parameters[argi].ntype == 'L' || method->parameters[argi].ntype == '['){
                        PYJAVA_ENVCALL(env,DeleteLocalRef,jargs[argi].l);
                    }
                }
                pyjava_free(jargs);
                break;
            }
        }
        if (err)
            continue;

        PyObject * ret = method->callHelper(env,method->methodid,obj?obj->obj:NULL,type->klass,jargs);

        for (Py_ssize_t argi = 0;argi<argcount;argi++){
            if (method->parameters[argi].ntype == 'L' || method->parameters[argi].ntype == '['){
                PYJAVA_ENVCALL(env,DeleteLocalRef,jargs[argi].l);
            }
        }
        pyjava_free(jargs);

        return ret;

    }

    PyErr_SetString(PyExc_LookupError,"no method found");
    return NULL;

}

int pyjava_hasFunction(JNIEnv * env, PyObject * _obj,const char * name){
    (void) env;
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return 0;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            type=(PyJavaType*)_obj->ob_type;
            obj=(PyJavaObject*)_obj;
        } else {
            PyErr_BadArgument();
            return 0;
        }
    }

    if (!name){
        PyErr_BadArgument();
        return 0;
    }

    const int namehash = pyjava_cstring_hash(name);

    PyJavaMethod * _method = type->methods[(unsigned)namehash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
    while (_method){
        PyJavaMethod * method = _method;
        _method = _method->next;
        if (strcmp(method->name,name)){
            continue;
        }
        if (!method->modifiers.isStatic && !obj){
            continue;
        }

        return 1;

    }

    return 0;

}


PyObject * pyjava_getField(JNIEnv * env, PyObject * _obj,const char * name){
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            type=(PyJavaType*)_obj->ob_type;
            obj=(PyJavaObject*)_obj;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    }

    if (!name){
        PyErr_BadArgument();
        return NULL;
    }

    const int namehash = pyjava_cstring_hash(name);


    PyJavaField * _field = type->fields[(unsigned)namehash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
    while (_field){
        PyJavaField * field = _field;
        _field = _field->next;
        if (strcmp(field->name,name)){
            continue;
        }
        if (!field->modifiers.isStatic && !obj){
            continue;
        }

        if (!field->getter){
            PyErr_SetString(PyExc_NotImplementedError,"access to this java field is not yet implemented");
            return NULL;
        }

        return field->getter(env,field->fieldid,obj?obj->obj:NULL,type->klass);
    }

    PyErr_SetString(PyExc_AttributeError,name);
    return NULL;

}

void pyjava_setField(JNIEnv * env, PyObject * _obj,const char * name,PyObject * val){
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            type=(PyJavaType*)_obj->ob_type;
            obj=(PyJavaObject*)_obj;
        } else {
            PyErr_BadArgument();
            return;
        }
    }

    if (!name){
        PyErr_BadArgument();
        return;
    }

    const int namehash = pyjava_cstring_hash(name);


    PyJavaField * _field = type->fields[(unsigned)namehash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
    while (_field){
        PyJavaField * field = _field;
        _field = _field->next;
        if (strcmp(field->name,name)){
            continue;
        }
        if (!field->modifiers.isStatic && !obj){
            continue;
        }

        if (!field->setter){
            PyErr_SetString(PyExc_NotImplementedError,"access to this java field is not yet implemented");
            return;
        }

        jvalue jval;
        jval.l = NULL;

        if (pyjava_asJObject(env,val,field->type,field->ntype,&jval)){
            field->setter(env,field->fieldid,obj?obj->obj:NULL,type->klass,&jval);
            return;
        }
    }

    PyErr_SetString(PyExc_AttributeError,name);
    return;
}

void pyjava_conversion_initType(JNIEnv * env,PyJavaType * type);

static int _pyjava_ptrinlist(PyObject * list,PyObject * ptr){
    for (Py_ssize_t i = 0;i<PyList_Size(list);i++){
        if (PyList_GET_ITEM(list,i) == ptr){
            return 1;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
/// copied function from cpython:
///
/// "Copyright (c) 2001-2017 Python Software Foundation; All Rights Reserved"
///
///
static int
add_subclass(PyTypeObject *base, PyTypeObject *type)
{
    int result = -1;
    PyObject *dict, *key, *newobj;

    dict = base->tp_subclasses;
    if (dict == NULL) {
        base->tp_subclasses = dict = PyDict_New();
        if (dict == NULL)
            return -1;
    }
    assert(PyDict_CheckExact(dict));
    key = PyLong_FromVoidPtr((void *) type);
    if (key == NULL)
        return -1;
    newobj = PyWeakref_NewRef((PyObject *)type, NULL);
    if (newobj != NULL) {
        result = PyDict_SetItem(dict, key, newobj);
        Py_DECREF(newobj);
    }
    Py_DECREF(key);
    return result;
}
///
///
///
///
/// end of copied code from cpython
//////////////////////////////////////////////////////////////////////////

static void _pyjava_inherit_decorator(PyJavaDecoration * dst,PyJavaDecoration * src){
    if (!dst->tp_call && src->tp_call){
        Py_IncRef(src->tp_call);
        dst->tp_call = src->tp_call;
        dst->inherited.tp_call = 1;
    }
    if (!dst->tp_getattro && src->tp_getattro){
        Py_IncRef(src->tp_getattro);
        dst->tp_getattro = src->tp_getattro;
        dst->inherited.tp_getattro = 1;
    }
    if (!dst->tp_setattro && src->tp_setattro){
        Py_IncRef(src->tp_setattro);
        dst->tp_setattro = src->tp_setattro;
        dst->inherited.tp_setattro = 1;
    }
}

static void _pyjava_inherit_decorators(PyJavaType * type){
    if (type->decoration.inherited.tp_call && type->decoration.tp_call){
        Py_DecRef(type->decoration.tp_call);
        type->decoration.tp_call = NULL;
        type->decoration.inherited.tp_call = 0;
    }
    if (type->decoration.inherited.tp_getattro && type->decoration.tp_getattro){
        Py_DecRef(type->decoration.tp_getattro);
        type->decoration.tp_getattro = NULL;
        type->decoration.inherited.tp_getattro = 0;
    }
    if (type->decoration.inherited.tp_setattro && type->decoration.tp_setattro){
        Py_DecRef(type->decoration.tp_setattro);
        type->decoration.tp_setattro = NULL;
        type->decoration.inherited.tp_setattro = 0;
    }

    if (type->pto.tp_base){
        if (pyjava_isJavaClass(type->pto.tp_base)){
            _pyjava_inherit_decorator(&type->decoration,&((PyJavaType*)type->pto.tp_base)->decoration);
        }
    }
    if (type->pto.tp_bases){
        if (PyTuple_CheckExact(type->pto.tp_bases)){
            const Py_ssize_t size = PyTuple_Size(type->pto.tp_bases);
            for (Py_ssize_t i = 0; i<size;i++){
                if (pyjava_isJavaClass((PyTypeObject*)(PyTuple_GET_ITEM(type->pto.tp_bases,i)))){
                    _pyjava_inherit_decorator(&type->decoration,&((PyJavaType*)(PyTuple_GET_ITEM(type->pto.tp_bases,i)))->decoration);
                }
            }
        }
    }
}

void pyjava_inherit_decorators(PyTypeObject * type){
    if (pyjava_isJavaClass(type)){
        _pyjava_inherit_decorators((PyJavaType*)type);
    }
}

PyTypeObject * pyjava_classAsType(JNIEnv * env,jclass klass){

	if (!klass){
		return NULL;
	}

    //find existing class wrapper
    {
        PyJavaType * type = pyjava_typecache_find(env,klass);
        if (type){
            return (PyTypeObject*)type;
        }
    }

    // build new wrapper
    PyJavaType * const ret = (PyJavaType*) pyjava_malloc(sizeof(PyJavaType));

    if (!ret){
        PYJAVA_SOFTASSERT(ret);
        return NULL;
    }

    memset(ret,0,sizeof(PyJavaType));

    // extract class name
    char * name = strcpy((char*)pyjava_malloc(sizeof(char)*8),"UNKNOWN");
    jmethodID class_getName = 0;
    {
        jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
        PYJAVA_SOFTASSERT(klassklass);
        if (klassklass){
            class_getName = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
            PYJAVA_SOFTASSERT(class_getName);
            PYJAVA_IGNORE_EXCEPTION(env);
            jmethodID tostring = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"toString","()Ljava/lang/String;");
            PYJAVA_SOFTASSERT(tostring);
            PYJAVA_IGNORE_EXCEPTION(env);
            jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,tostring);
            PYJAVA_SOFTASSERT(jname);
            PYJAVA_IGNORE_EXCEPTION(env);
            if (jname){
                const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
                PYJAVA_SOFTASSERT(tmp);
                int off = 0;
                //if (tmp[0]=='c' && tmp[1]=='l' && tmp[2]=='a' && tmp[3]=='s' && tmp[4]=='s' && tmp[5]==' ')
                //    off = 6;
                pyjava_free(name);
                name = strcpy((char*)pyjava_malloc(sizeof(char)*strlen(tmp+off)),tmp+off);
                PYJAVA_SOFTASSERT(name);
                PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
                PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
            }
            PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
        }
    }

    // create type object
    {

        // create temporary type object to initialize the type object
        {
            PyTypeObject retinit = {
                PyVarObject_HEAD_INIT(NULL, 0)
                name,
                sizeof(PyJavaObject),
                0,
                (destructor)pyjava_type_dealloc, /* tp_dealloc */
                0,                         /* tp_print */
                0,                         /* tp_getattr */
                0,                         /* tp_setattr */
                0,                         /* tp_reserved */
                (PyObject*(*)(PyObject*))pyjava_type_repr,                         /* tp_repr */
                0,                         /* tp_as_number */
                0,                         /* tp_as_sequence */
                0,                         /* tp_as_mapping */
                (Py_hash_t(*)(PyObject*))pyjava_type_hash,          /* tp_hash  */
                (ternaryfunc)pyjava_type_call,                         /* tp_call */
                0,                         /* tp_str */
                pyjava_type_getattro,                         /* tp_getattro */
                pyjava_type_setattro,                         /* tp_setattro */
                0,                         /* tp_as_buffer */
                0,        /* tp_flags */
                "java class",              /* tp_doc */
                0,                         /* tp_traverse */
                0,                         /* tp_clear */
                pyjava_type_richcompare,   /* tp_richcompare */
                0,                         /* tp_weaklistoffset */
                0,                         /* tp_iter */
                0,                         /* tp_iternext */
                0,                         /* tp_methods */
                0,                         /* tp_members */
                0,                         /* tp_getset */
                0,                         /* tp_base */
                0,                         /* tp_dict */
                0,                         /* tp_descr_get */
                0,                         /* tp_descr_set */
                0,                         /* tp_dictoffset */
                (initproc)pyjava_type_init,      /* tp_init */
                (allocfunc)pyjava_type_alloc,                         /* tp_alloc */
                (newfunc)pyjava_type_new,                         /* tp_new */
                0,                         /* tp_free*/
                0,                         /* tp_is_gc For PyObject_IS_GC */
                0,                         /* tp_bases */
                0,                         /* tp_mro method resolution order */
                0,                         /* tp_cache */
                0,                         /* tp_subclasses  */
                0,                         /* tp_weaklist */
                0,                         /* tp_del */
                0,                         /* tp_version_tag */
                0                          /* tp_finalize */
            };
            memcpy(&ret->pto,&retinit,sizeof(PyTypeObject));
        }

        ret->klass = PYJAVA_ENVCALL(env,NewGlobalRef,klass);
        PYJAVA_SOFTASSERT(ret->klass);
        ret->arrayklass = pyjava_get_array_sub_Type(env,ret->klass);
        if (ret->arrayklass){
            ret->arrayntype = pyjava_getNType(env,ret->arrayklass);
        }
        ret->toString = PYJAVA_ENVCALL(env,GetMethodID,klass,"toString","()Ljava/lang/String;");
        PYJAVA_SOFTASSERT(ret->toString);
        PYJAVA_IGNORE_EXCEPTION(env);
        ret->hashCode = PYJAVA_ENVCALL(env,GetMethodID,klass,"hashCode","()I");
        PYJAVA_SOFTASSERT(ret->hashCode);
        PYJAVA_IGNORE_EXCEPTION(env);
        ret->class_getName = class_getName;
        ret->dir = PyList_New(0);
        PYJAVA_ASSERT(ret->dir);
        ret->pto.tp_dict = PyDict_New();
        ret->decoration.inherited.tp_call = 1;
        ret->decoration.inherited.tp_getattro = 1;
        ret->decoration.inherited.tp_setattro = 1;
        PYJAVA_ASSERT(ret->pto.tp_dict);
        // parent class
        {
            jclass klass = ret->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
                PYJAVA_SOFTASSERT(klassklass);
                PYJAVA_IGNORE_EXCEPTION(env);
                if (klassklass){
                    PyObject * lbases = PyList_New(0);
                    PYJAVA_SOFTASSERT(lbases);
                    // super class
                    jmethodID getsuper = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getSuperclass","()Ljava/lang/Class;");
                    PYJAVA_SOFTASSERT(getsuper);
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (getsuper){
                        jclass super = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getsuper);
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (super){
                            PyObject * base = (PyObject*)pyjava_classAsType(env,super);
                            PYJAVA_SOFTASSERT(base);
                            if (base){
                                ret->pto.tp_base = (PyTypeObject*)base;
                            }
                            PYJAVA_ENVCALL(env,DeleteLocalRef,super);
                        }
                    }
                    //add interfaces
                    jmethodID getif = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getInterfaces","()[Ljava/lang/Class;");
                    PYJAVA_SOFTASSERT(getif);
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (getif){
                        jclass ifs = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getif);
                        PYJAVA_SOFTASSERT(ifs);
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (ifs){
                            for (jsize i = 0;i<PYJAVA_ENVCALL(env,GetArrayLength,ifs);i++){
                                PYJAVA_IGNORE_EXCEPTION(env);
                                jclass ifc = PYJAVA_ENVCALL(env,GetObjectArrayElement,ifs,i);
                                PYJAVA_SOFTASSERT(ifc);
                                PYJAVA_IGNORE_EXCEPTION(env);
                                if (ifc){
                                    PyObject * base = (PyObject*)pyjava_classAsType(env,ifc);
                                    if (base){
                                        PyList_Append(lbases,base);
                                        Py_DecRef(base);
                                    }
                                }
                            }
                            PYJAVA_ENVCALL(env,DeleteLocalRef,ifs);
                        }
                    }
                    ret->pto.tp_bases = lbases;
                    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                }
            }
        }
        // read constructors
        {
            jclass klass = ret->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
                PYJAVA_IGNORE_EXCEPTION(env);
                if (klassklass){
                    jclass methodklass = PYJAVA_ENVCALL(env,FindClass,"java/lang/reflect/Constructor");
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (methodklass){
                        jmethodID getMethods = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getConstructors","()[Ljava/lang/reflect/Constructor;");
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (getMethods){
                            jobjectArray methods = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getMethods);
                            PYJAVA_IGNORE_EXCEPTION(env);
                            if (methods){
                                jmethodID getModifiers = NULL;
                                jmethodID getParameterTypes = NULL;
                                const jsize methodcount = PYJAVA_ENVCALL(env,GetArrayLength,methods);
                                PYJAVA_IGNORE_EXCEPTION(env);
                                for (jsize funci =0;funci<methodcount;funci++){
                                    jobject method = PYJAVA_ENVCALL(env,GetObjectArrayElement,methods,funci);
                                    PYJAVA_IGNORE_EXCEPTION(env);
                                    if (method){
                                        // do some initialization
                                        {
                                            if (!getParameterTypes){
                                                getParameterTypes = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getParameterTypes","()[Ljava/lang/Class;");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                            if (!getModifiers){
                                                getModifiers = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getModifiers","()I");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                        }
                                        PyJavaMethod * methdef = (PyJavaMethod*)pyjava_malloc(sizeof(PyJavaMethod));
                                        memset(methdef,0,sizeof(PyJavaMethod));
                                        methdef->methodid = PYJAVA_ENVCALL(env,FromReflectedMethod,method);
                                        PYJAVA_IGNORE_EXCEPTION(env);
                                        //function name
                                        {
                                            methdef->name = pyjava_dedupstaticstr("<init>");
                                        }
                                        //modifiers
                                        {
                                            methdef->modifiers.all = PYJAVA_ENVCALL(env,CallIntMethod,method,getModifiers);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            methdef->modifiers.isStatic = 1;
                                        }
                                        //add arguments
                                        {
                                            jobjectArray argst = PYJAVA_ENVCALL(env,CallObjectMethod,method,getParameterTypes);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (argst){
                                                const jsize argcount = PYJAVA_ENVCALL(env,GetArrayLength,argst);
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                                methdef->parametercount = argcount;
                                                methdef->parameters = (PyJavaParameter*)pyjava_malloc(argcount*sizeof(PyJavaParameter));
                                                memset(methdef->parameters,0,argcount*sizeof(PyJavaParameter));
                                                for (jsize argi =0;argi<argcount;argi++){
                                                    jclass argt = PYJAVA_ENVCALL(env,GetObjectArrayElement,argst,argi);
                                                    PYJAVA_IGNORE_EXCEPTION(env);
                                                    if (argt){
                                                        methdef->parameters[argi].klass = PYJAVA_ENVCALL(env,NewGlobalRef,argt);
                                                        methdef->parameters[argi].ntype = pyjava_getNType(env,methdef->parameters[argi].klass);
                                                        //TODO name is not initialized; change code to add that
                                                        PYJAVA_ENVCALL(env,DeleteLocalRef,argt);
                                                    }
                                                }
                                                PYJAVA_ENVCALL(env,DeleteLocalRef,argst);
                                            }
                                        }
                                        //append return type
                                        {
                                            methdef->returnType = PYJAVA_ENVCALL(env,NewGlobalRef,klass);
                                            methdef->returnNType = 'L';
                                        }
                                        methdef->callHelper = pyjava_callHelperConstructor;

                                        {
                                            int mhash = pyjava_cstring_hash(methdef->name);
                                            methdef->next = ret->methods[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
                                            ret->methods[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)] = methdef;
                                        }

                                        PYJAVA_ENVCALL(env,DeleteLocalRef,method);
                                    }
                                }
                                PYJAVA_ENVCALL(env,DeleteLocalRef,methods);
                            }
                        }
                        PYJAVA_ENVCALL(env,DeleteLocalRef,methodklass);
                    }
                    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                }
            }
        }
        // read methods
        {
            jclass klass = ret->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
                PYJAVA_IGNORE_EXCEPTION(env);
                if (klassklass){
                    jclass methodklass = PYJAVA_ENVCALL(env,FindClass,"java/lang/reflect/Method");
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (methodklass){
                        jmethodID getMethods = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getMethods","()[Ljava/lang/reflect/Method;");
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (getMethods){
                            jobjectArray methods = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getMethods);
                            PYJAVA_IGNORE_EXCEPTION(env);
                            if (methods){
                                jmethodID getMethodName = NULL;
                                jmethodID getModifiers = NULL;
                                jmethodID getParameterTypes = NULL;
                                const jsize methodcount = PYJAVA_ENVCALL(env,GetArrayLength,methods);
                                PYJAVA_IGNORE_EXCEPTION(env);
                                for (jsize funci =0;funci<methodcount;funci++){
                                    jobject method = PYJAVA_ENVCALL(env,GetObjectArrayElement,methods,funci);
                                    PYJAVA_IGNORE_EXCEPTION(env);
                                    if (method){
                                        // do some initialization
                                        {
                                            if (!getMethodName){
                                                getMethodName = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getName","()Ljava/lang/String;");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                            if (!getParameterTypes){
                                                getParameterTypes = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getParameterTypes","()[Ljava/lang/Class;");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                            if (!getModifiers){
                                                getModifiers = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getModifiers","()I");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                        }
                                        PyJavaMethod * methdef = (PyJavaMethod*)pyjava_malloc(sizeof(PyJavaMethod));
                                        memset(methdef,0,sizeof(PyJavaMethod));
                                        methdef->methodid = PYJAVA_ENVCALL(env,FromReflectedMethod,method);
                                        PYJAVA_IGNORE_EXCEPTION(env);
                                        //function name
                                        {
                                            jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,method,getMethodName);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (jname){
                                                {
                                                    const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
                                                    PYJAVA_IGNORE_EXCEPTION(env);
                                                    if (tmp){
                                                        methdef->name = pyjava_dedupstaticstr(tmp);
                                                    }
                                                    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
                                                }
                                                PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
                                            }
                                        }
                                        //modifiers
                                        {
                                            methdef->modifiers.all = PYJAVA_ENVCALL(env,CallIntMethod,method,getModifiers);
                                            PYJAVA_IGNORE_EXCEPTION(env);

                                        }
                                        //add arguments
                                        {
                                            jobjectArray argst = PYJAVA_ENVCALL(env,CallObjectMethod,method,getParameterTypes);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (argst){
                                                const jsize argcount = PYJAVA_ENVCALL(env,GetArrayLength,argst);
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                                methdef->parametercount = argcount;
                                                methdef->parameters = (PyJavaParameter*)pyjava_malloc(argcount*sizeof(PyJavaParameter));
                                                memset(methdef->parameters,0,argcount*sizeof(PyJavaParameter));
                                                for (jsize argi =0;argi<argcount;argi++){
                                                    jclass argt = PYJAVA_ENVCALL(env,GetObjectArrayElement,argst,argi);
                                                    PYJAVA_IGNORE_EXCEPTION(env);
                                                    if (argt){
                                                        methdef->parameters[argi].klass = PYJAVA_ENVCALL(env,NewGlobalRef,argt);
                                                        PYJAVA_IGNORE_EXCEPTION(env);
                                                        methdef->parameters[argi].ntype = pyjava_getNType(env,methdef->parameters[argi].klass);
                                                        //TODO name is not initialized; change code to add that
                                                        PYJAVA_ENVCALL(env,DeleteLocalRef,argt);
                                                    }
                                                }
                                                PYJAVA_ENVCALL(env,DeleteLocalRef,argst);
                                            }
                                        }
                                        //append return type
                                        {
                                            jmethodID getReturnType = PYJAVA_ENVCALL(env,GetMethodID,methodklass,"getReturnType","()Ljava/lang/Class;");
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (getReturnType){
                                                jclass retclass = PYJAVA_ENVCALL(env,CallObjectMethod,method,getReturnType);
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                                if (retclass){
                                                    methdef->returnType = PYJAVA_ENVCALL(env,NewGlobalRef,retclass);
                                                    methdef->returnNType = pyjava_getNType(env,retclass);
                                                    if (methdef->modifiers.isStatic){
                                                        switch (methdef->returnNType){
                                                            case 'V': methdef->callHelper = &pyjava_callHelperStaticVoid;break;
                                                            case 'Z': methdef->callHelper = &pyjava_callHelperStaticBool;break;
                                                            case 'B': methdef->callHelper = &pyjava_callHelperStaticByte;break;
                                                            case 'C': methdef->callHelper = &pyjava_callHelperStaticChar;break;
                                                            case 'S': methdef->callHelper = &pyjava_callHelperStaticShort;break;
                                                            case 'I': methdef->callHelper = &pyjava_callHelperStaticInt;break;
                                                            case 'J': methdef->callHelper = &pyjava_callHelperStaticLong;break;
                                                            case 'F': methdef->callHelper = &pyjava_callHelperStaticFloat;break;
                                                            case 'D': methdef->callHelper = &pyjava_callHelperStaticDouble;break;
                                                            default : methdef->callHelper = &pyjava_callHelperStaticObject;break;

                                                        }
                                                    } else {
                                                        switch (methdef->returnNType){
                                                            case 'V': methdef->callHelper = &pyjava_callHelperVoid;break;
                                                            case 'Z': methdef->callHelper = &pyjava_callHelperBool;break;
                                                            case 'B': methdef->callHelper = &pyjava_callHelperByte;break;
                                                            case 'C': methdef->callHelper = &pyjava_callHelperChar;break;
                                                            case 'S': methdef->callHelper = &pyjava_callHelperShort;break;
                                                            case 'I': methdef->callHelper = &pyjava_callHelperInt;break;
                                                            case 'J': methdef->callHelper = &pyjava_callHelperLong;break;
                                                            case 'F': methdef->callHelper = &pyjava_callHelperFloat;break;
                                                            case 'D': methdef->callHelper = &pyjava_callHelperDouble;break;
                                                            default : methdef->callHelper = &pyjava_callHelperObject;break;

                                                        }
                                                    }
                                                    PYJAVA_ENVCALL(env,DeleteLocalRef,retclass);
                                                }
                                            }
                                        }

                                        // add method to type
                                        {
                                            int mhash = pyjava_cstring_hash(methdef->name);
                                            methdef->next = ret->methods[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
                                            ret->methods[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)] = methdef;
                                        }

                                        // add dict entry
                                        {
                                            PyObject * str = PyUnicode_FromString(methdef->name);
                                            int found = 0;
                                            for(Py_ssize_t i = 0;i<PyList_Size(ret->dir);i++){
                                                if (!PyUnicode_Compare(str,PyList_GET_ITEM(ret->dir,i))){
                                                    found = 1;
                                                    break;
                                                }
                                            }
                                            if (!found){
                                                PyList_Append(ret->dir,str);
                                            }
                                            Py_DecRef(str);
                                        }

                                        PYJAVA_ENVCALL(env,DeleteLocalRef,method);
                                    }
                                    if (PYJAVA_ENVCALL(env,ExceptionCheck)){
                                        PYJAVA_ENVCALL(env,ExceptionDescribe);
                                        PYJAVA_ENVCALL(env,ExceptionClear);
                                    }
                                }
                                PYJAVA_ENVCALL(env,DeleteLocalRef,methods);
                            }
                        }
                        PYJAVA_ENVCALL(env,DeleteLocalRef,methodklass);
                    }
                    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                }
            }
        }
        // read fields
        {
            jclass klass = ret->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
                PYJAVA_IGNORE_EXCEPTION(env);
                if (klassklass){
                    jclass fieldklass = PYJAVA_ENVCALL(env,FindClass,"java/lang/reflect/Field");
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (fieldklass){
                        jmethodID getFields = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getFields","()[Ljava/lang/reflect/Field;");
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (getFields){
                            jobjectArray fields = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getFields);
                            PYJAVA_IGNORE_EXCEPTION(env);
                            if (fields){
                                jmethodID getFieldName = NULL;
                                jmethodID getModifiers = NULL;
                                jmethodID getType = NULL;
                                const jsize fieldcount = PYJAVA_ENVCALL(env,GetArrayLength,fields);
                                PYJAVA_IGNORE_EXCEPTION(env);
                                for (jsize funci =0;funci<fieldcount;funci++){
                                    jobject field = PYJAVA_ENVCALL(env,GetObjectArrayElement,fields,funci);
                                    PYJAVA_IGNORE_EXCEPTION(env);
                                    if (field){
                                        // do some initialization
                                        {
                                            if (!getFieldName){
                                                getFieldName = PYJAVA_ENVCALL(env,GetMethodID,fieldklass,"getName","()Ljava/lang/String;");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                            if (!getModifiers){
                                                getModifiers = PYJAVA_ENVCALL(env,GetMethodID,fieldklass,"getModifiers","()I");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                            if (!getType){
                                                getType = PYJAVA_ENVCALL(env,GetMethodID,fieldklass,"getType","()Ljava/lang/Class;");
                                                PYJAVA_IGNORE_EXCEPTION(env);
                                            }
                                        }
                                        PyJavaField * fielddef = (PyJavaField*)pyjava_malloc(sizeof(PyJavaField));
                                        memset(fielddef,0,sizeof(PyJavaField));
                                        fielddef->fieldid = PYJAVA_ENVCALL(env,FromReflectedField,field);
                                        PYJAVA_IGNORE_EXCEPTION(env);
                                        //name
                                        {
                                            jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,field,getFieldName);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (jname){
                                                {
                                                    const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
                                                    if (tmp){
                                                        fielddef->name = pyjava_dedupstaticstr(tmp);
                                                    }
                                                    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
                                                }
                                                PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
                                            }
                                        }
                                        //modifiers
                                        {
                                            fielddef->modifiers.all = PYJAVA_ENVCALL(env,CallIntMethod,field,getModifiers);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                        }
                                        // type
                                        {
                                            jclass retclass = PYJAVA_ENVCALL(env,CallObjectMethod,field,getType);
                                            PYJAVA_IGNORE_EXCEPTION(env);
                                            if (retclass){
                                                fielddef->type = PYJAVA_ENVCALL(env,NewGlobalRef,retclass);
                                                fielddef->ntype = pyjava_getNType(env,retclass);
                                                if (fielddef->modifiers.isStatic){
                                                    switch (fielddef->ntype){
                                                    case 'V':
                                                        fielddef->getter = NULL;
                                                        fielddef->setter = NULL;
                                                        break;
                                                    case 'Z':
                                                        fielddef->getter = &pyjava_getStaticFieldBool;
                                                        fielddef->setter = &pyjava_setStaticFieldBool;
                                                        break;
                                                    case 'B':
                                                        fielddef->getter = &pyjava_getStaticFieldByte;
                                                        fielddef->setter = &pyjava_setStaticFieldByte;
                                                        break;
                                                    case 'C':
                                                        fielddef->getter = &pyjava_getStaticFieldChar;
                                                        fielddef->setter = &pyjava_setStaticFieldChar;
                                                        break;
                                                    case 'S':
                                                        fielddef->getter = &pyjava_getStaticFieldShort;
                                                        fielddef->setter = &pyjava_setStaticFieldShort;
                                                        break;
                                                    case 'I':
                                                        fielddef->getter = &pyjava_getStaticFieldInt;
                                                        fielddef->setter = &pyjava_setStaticFieldInt;
                                                        break;
                                                    case 'J':
                                                        fielddef->getter = &pyjava_getStaticFieldLong;
                                                        fielddef->setter = &pyjava_setStaticFieldLong;
                                                        break;
                                                    case 'F':
                                                        fielddef->getter = &pyjava_getStaticFieldFloat;
                                                        fielddef->setter = &pyjava_setStaticFieldFloat;
                                                        break;
                                                    case 'D':
                                                        fielddef->getter = &pyjava_getStaticFieldDouble;
                                                        fielddef->setter = &pyjava_setStaticFieldDouble;
                                                        break;
                                                    default :
                                                        fielddef->getter = &pyjava_getStaticFieldObject;
                                                        fielddef->setter = &pyjava_setStaticFieldObject;
                                                        break;

                                                    }
                                                } else {
                                                    switch (fielddef->ntype){
                                                    case 'V':
                                                        fielddef->getter = NULL;
                                                        fielddef->setter = NULL;
                                                        break;
                                                    case 'Z':
                                                        fielddef->getter = &pyjava_getFieldBool;
                                                        fielddef->setter = &pyjava_setFieldBool;
                                                        break;
                                                    case 'B':
                                                        fielddef->getter = &pyjava_getFieldByte;
                                                        fielddef->setter = &pyjava_setFieldByte;
                                                        break;
                                                    case 'C':
                                                        fielddef->getter = &pyjava_getFieldChar;
                                                        fielddef->setter = &pyjava_setFieldChar;
                                                        break;
                                                    case 'S':
                                                        fielddef->getter = &pyjava_getFieldShort;
                                                        fielddef->setter = &pyjava_setFieldShort;
                                                        break;
                                                    case 'I':
                                                        fielddef->getter = &pyjava_getFieldInt;
                                                        fielddef->setter = &pyjava_setFieldInt;
                                                        break;
                                                    case 'J':
                                                        fielddef->getter = &pyjava_getFieldLong;
                                                        fielddef->setter = &pyjava_setFieldLong;
                                                        break;
                                                    case 'F':
                                                        fielddef->getter = &pyjava_getFieldFloat;
                                                        fielddef->setter = &pyjava_setFieldFloat;
                                                        break;
                                                    case 'D':
                                                        fielddef->getter = &pyjava_getFieldDouble;
                                                        fielddef->setter = &pyjava_setFieldDouble;
                                                        break;
                                                    default :
                                                        fielddef->getter = &pyjava_getFieldObject;
                                                        fielddef->setter = &pyjava_setFieldObject;
                                                        break;
                                                    }
                                                }
                                                PYJAVA_ENVCALL(env,DeleteLocalRef,retclass);
                                            }
                                        }

                                        {
                                            int mhash = pyjava_cstring_hash(fielddef->name);
                                            fielddef->next = ret->fields[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)];
                                            ret->fields[(unsigned)mhash%(PYJAVA_SYMBOL_BUCKET_COUNT)] = fielddef;
                                        }

                                        // add dict entry
                                        {
                                            PyObject * str = PyUnicode_FromString(fielddef->name);
                                            if (str){
                                                int found = 0;
                                                for(Py_ssize_t i = 0;i<PyList_Size(ret->dir);i++){
                                                    if (!PyUnicode_Compare(str,PyList_GET_ITEM(ret->dir,i))){
                                                        found = 1;
                                                        break;
                                                    }
                                                }
                                                if (!found){
                                                    PyList_Append(ret->dir,str);
                                                }
                                                Py_DecRef(str);
                                            }
                                        }

                                        PYJAVA_ENVCALL(env,DeleteLocalRef,field);
                                    }
                                    if (PYJAVA_ENVCALL(env,ExceptionCheck)){
                                        PYJAVA_ENVCALL(env,ExceptionClear);
                                    }
                                }
                                PYJAVA_ENVCALL(env,DeleteLocalRef,fields);
                            }
                        }
                        PYJAVA_ENVCALL(env,DeleteLocalRef,fieldklass);
                    }
                    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                }
            }
        }

        //convert dir to tuple
        {
            PyObject * del = ret->dir;
            ret->dir = PyList_AsTuple(del);
            Py_DecRef(del);
        }

        {
            PyObject * module = pyjava_getModule();
            if (module){
                PyObject * globals = PyModule_GetDict(module);
                PyObject * locals = PyDict_New();

                ret->pto.tp_dict = PyDict_New();
                {
                    for (Py_ssize_t i = 0;i<PyTuple_Size(ret->dir);i++){
                        PyDict_SetItem(ret->pto.tp_dict,PyTuple_GET_ITEM(ret->dir,i),Py_None);
                    }
                }
                {
                    const char * def =
                            "tmp = (lambda syms: lambda o: syms(o))(symbols)\n"
                            ;
                    PyRun_String(def, Py_single_input, globals, locals);
                    PyObject * d = PyRun_String("tmp", Py_eval_input, globals, locals);
                    PyObject * dir = PyUnicode_FromString("__dir__");
                    PyDict_SetItem(ret->pto.tp_dict,dir,d);
                    Py_DecRef(dir);
                    Py_DecRef(d);
                }
                Py_DecRef(locals);
                Py_DecRef(module);
            }

        }

        pyjava_init_type_extensions(env,ret);

        //fake mro (pt1)
        PyObject * bases = ret->pto.tp_bases;
        if (ret->pto.tp_base){
            ret->pto.tp_bases = PyTuple_New(1);
            Py_IncRef((PyObject*)ret->pto.tp_base);
            PyTuple_SetItem(ret->pto.tp_bases,0,(PyObject*)ret->pto.tp_base);

        } else {
            ret->pto.tp_bases = NULL;
        }

        if (PyType_Ready(&(ret->pto))){
            //TODO cleanup
            return NULL;
        }
        //fake mro (pt2)
        if (PyList_Size(bases) > 0){
            PyObject * nbases = PySequence_List(ret->pto.tp_bases);
            PyObject * nmro = PySequence_List(ret->pto.tp_mro);
            for (Py_ssize_t i = 0;i<PyList_Size(bases);i++){
                PyTypeObject * curbase = (PyTypeObject *) PyList_GET_ITEM(bases,i);
                if (!_pyjava_ptrinlist(nbases,(PyObject*)curbase)){
                    PyList_Append(nbases,(PyObject*)curbase);
                    add_subclass(curbase,&(ret->pto));
                    if (!_pyjava_ptrinlist(nmro,(PyObject*)curbase)){
                        PyList_Append(nmro,(PyObject*)curbase);
                        for (Py_ssize_t j = 0; j < PyTuple_Size(curbase->tp_mro);j++){
                            if (!_pyjava_ptrinlist(nmro,PyTuple_GET_ITEM(curbase->tp_mro,j))){
                                PyList_Append(nmro,PyTuple_GET_ITEM(curbase->tp_mro,j));
                            }
                        }
                    }
                }
            }
            Py_DecRef(ret->pto.tp_mro);
            ret->pto.tp_mro = PyList_AsTuple(nmro);
            Py_DecRef(nmro);
            Py_DecRef(ret->pto.tp_bases);
            ret->pto.tp_bases = PyList_AsTuple(nbases);
            Py_DecRef(nbases);
        }

        Py_DecRef(bases);

        _pyjava_inherit_decorators(ret);

        pyjava_typecache_register(env,ret);

        pyjava_conversion_initType(env,ret);

    }

    if (PyErr_Occurred())
        PyErr_Print();

    Py_IncRef((PyObject*)ret);
    return (PyTypeObject*)ret;
	
}

PyTypeObject * pyjava_classNameAsType(JNIEnv * env,PyObject * classloader,const char * classname){
	
	jclass klass = NULL;

    if (classloader){
        //TODO
    } else {
        PYJAVA_YIELD_GIL(state);
        klass = PYJAVA_ENVCALL(env,FindClass, classname);
        PYJAVA_RESTORE_GIL(state);
        if (pyjava_exception_java2python(env)){
            klass = NULL;
        }

    }
	
	if (!klass){
		return NULL;
	}
	
    return pyjava_classAsType(env,klass);

}


#ifdef __cplusplus
}
#endif
