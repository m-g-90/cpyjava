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

#include "pyjava/type.h"
#include "pyjava/conversion.h"
#include "pyjava/memory.h"
#include "pyjava/type_cache.h"

#ifdef __cplusplus
extern "C"{
#endif

pyjava_type_dealloc(PyJavaObject * self)
{
    jobject obj = self->obj;
    self->obj = NULL;
    Py_TYPE(self)->tp_free((PyObject*)self);
    PYJAVA_YIELD_GIL(gstate);
    PYJAVA_START_JAVA(env);
    PYJAVA_ENVCALL(env,DeleteGlobalRef,obj);
    PYJAVA_END_JAVA(env);
    PYJAVA_RESTORE_GIL(gstate);
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

static PyObject * _pyjava_method_helper = NULL;
static PyObject* pyjava_type_getattro( PyObject* self, PyObject* pyname){
    if (!PyUnicode_CheckExact(pyname)){
        PyErr_SetString(PyExc_AttributeError,"java object only has string attributes");
        return NULL;
    }

    int method = 0;

    const char  *name = PyUnicode_AsUTF8(pyname);
    printf("getattro: %s",name);
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
            PyObject * ctx = PyDict_New();
            {
                PyObject* del = PyRun_String("import cpyjava", Py_single_input, PyEval_GetGlobals(), ctx);
                if (del){
                    Py_DecRef(del);
                }
            }
            if (!PyErr_Occurred()){
                _pyjava_method_helper = PyRun_String("lambda s,n: lambda *args,**kvargs: cpyjava.callMethod(s,n,args)", Py_eval_input, PyEval_GetGlobals(), ctx);
            }
        }
        if (_pyjava_method_helper){
            PyObject * args = PyTuple_New(2);
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

char pyjava_getNType(JNIEnv * env,jclass klass){
    char ret = 0;
    jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
    jmethodID toString = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,toString);
    PYJAVA_IGNORE_EXCEPTION(env);
    const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
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
    } else if (tmp[0]=='L'){
        ret = 'L';
    } else if (tmp[0]=='['){
        ret = '[';
    } else if (tmp[0]=='('){
        ret = 'M';
    } else {
        ret = 0;
    }
    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
    PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
    return ret;
}

static int pyjava_type_init(PyJavaObject *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

PyObject *pyjava_type_alloc(PyTypeObject *self, Py_ssize_t nitems){
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

    Py_ssize_t arglen = 0;
    if (args) {
        arglen = PyTuple_Size(args);
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

#define PYJAVA_TYPEMAP_SIZE (256*256)
static PyJavaType * typemap[PYJAVA_TYPEMAP_SIZE];
static PyJavaType * typeset[PYJAVA_TYPEMAP_SIZE];
static jint typeset_ptr_hash(void *ptr){
    return (jint)(((intptr_t)ptr)/sizeof(PyJavaType));
}

static jclass _pyjava_identityHash_system = NULL;
static jmethodID _pyjava_identityHash_system_identityHash = NULL;
static jint _pyjava_identityHash(JNIEnv * env,jobject obj){
	
    if (!_pyjava_identityHash_system){
        jclass system = PYJAVA_ENVCALL(env,FindClass, "java/lang/System"); //TODO store global reference
        _pyjava_identityHash_system = PYJAVA_ENVCALL(env,NewGlobalRef,system);
        PYJAVA_ENVCALL(env,DeleteLocalRef,system);
    }
	
    if (!_pyjava_identityHash_system){
        return 0;
	}
	
    if (!_pyjava_identityHash_system_identityHash){
        _pyjava_identityHash_system_identityHash = PYJAVA_ENVCALL(env,GetStaticMethodID,_pyjava_identityHash_system,"identityHashCode","(Ljava/lang/Object;)I");
    }

    if (!_pyjava_identityHash_system_identityHash){
        PyErr_Clear();
        return 0;
    }

    return PYJAVA_ENVCALL(env,CallStaticIntMethod,_pyjava_identityHash_system,_pyjava_identityHash_system_identityHash,obj);
	
}

int pyjava_isJavaClass(PyTypeObject * type){
    if (type){
        return (void*)type->tp_new == (void*)pyjava_type_new;
    } else {
        return 0;
    }
}

jclass _pyjava_getClass(JNIEnv * env, PyTypeObject * type){
    return PYJAVA_ENVCALL(env,NewLocalRef,((PyJavaType*)type)->klass);
}


#include "pyjava/type_helpers.hc"


PyObject * pyjava_callFunction(JNIEnv * env, PyObject * _obj,const char * name,PyObject * tuple){
    int isClass = 0;
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            isClass = 1;
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            isClass = 0;
            type=(PyJavaType*)_obj->ob_type;
            obj=(PyJavaObject*)_obj;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    }
    if (!PyTuple_CheckExact(tuple)){
        PyErr_BadArgument();
        return NULL;
    }

    if (!name){
        PyErr_BadArgument();
        return NULL;
    }

    const int namehash = pyjava_cstring_hash(name);

    Py_ssize_t argcount = PyTuple_Size(tuple);

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

        pyjava_free(jargs);

        return ret;

    }

    PyErr_SetString(PyExc_LookupError,"no method found");
    return NULL;

}

PyObject * pyjava_getField(JNIEnv * env, PyObject * _obj,const char * name){
    int isClass = 0;
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            isClass = 1;
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return NULL;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            isClass = 0;
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
        }

        return field->getter(env,field->fieldid,obj?obj->obj:NULL,type->klass);
    }

    PyErr_SetString(PyExc_AttributeError,name);
    return NULL;

}

void pyjava_setField(JNIEnv * env, PyObject * _obj,const char * name,PyObject * val){
    int isClass = 0;
    PyJavaType * type;
    PyJavaObject * obj;
    if (PyType_Check(_obj)){
        if (pyjava_isJavaClass((PyTypeObject*)_obj)){
            isClass = 1;
            type=(PyJavaType*)_obj;
            obj = NULL;
        } else {
            PyErr_BadArgument();
            return;
        }
    } else {
        if (pyjava_isJavaClass(_obj->ob_type)){
            isClass = 0;
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

static const char * _pyjava_class_getName(JNIEnv * env,jclass klass){
    jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
    jmethodID toString = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,toString);
    const char * ret = NULL;
    if (!pyjava_exception_java2python(env)){
        const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
        ret = strcpy((char*)malloc(sizeof(char)*strlen(tmp)+1),tmp);
        PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    }
    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
    return ret;
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
        return NULL;
    }

    memset(ret,0,sizeof(PyJavaType));

    // extract class name
    char * name = strcpy((char*)pyjava_malloc(sizeof(char)*8),"UNKNOWN");
    jmethodID class_getName = 0;
    {
        jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
        if (klassklass){
            class_getName = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
            PYJAVA_IGNORE_EXCEPTION(env);
            jmethodID tostring = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"toString","()Ljava/lang/String;");
            PYJAVA_IGNORE_EXCEPTION(env);
            jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,tostring);
            PYJAVA_IGNORE_EXCEPTION(env);
            if (jname){
                const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
                pyjava_free(name);
                name = strcpy((char*)pyjava_malloc(sizeof(char)*strlen(tmp)),tmp);
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
                0,                         /* tp_call */
                0,                         /* tp_str */
                pyjava_type_getattro,                         /* tp_getattro */
                pyjava_type_setattro,                         /* tp_setattro */
                0,                         /* tp_as_buffer */
                0,        /* tp_flags */
                "java class",              /* tp_doc */
                0,                         /* tp_traverse */
                0,                         /* tp_clear */
                0,                         /* tp_richcompare */
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
        ret->toString = PYJAVA_ENVCALL(env,GetMethodID,klass,"toString","()Ljava/lang/String;");
        PYJAVA_IGNORE_EXCEPTION(env);
        ret->hashCode = PYJAVA_ENVCALL(env,GetMethodID,klass,"hashCode","()I");
        PYJAVA_IGNORE_EXCEPTION(env);
        ret->class_getName = class_getName;
        ret->dir = PyList_New(0);
        // parent class
        {
            jclass klass = ret->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
                PYJAVA_IGNORE_EXCEPTION(env);
                if (klassklass){
                    PyObject * lbases = PyList_New(0);
                    // super class
                    jmethodID getsuper = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getSuperclass","()Ljava/lang/Class;");
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (getsuper){
                        jclass super = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getsuper);
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (super){
                            PyList_Append(lbases,(PyObject*)pyjava_classAsType(env,super));
                            PYJAVA_ENVCALL(env,DeleteLocalRef,super);
                        }
                    }
                    // TODO add interfaces
                    jmethodID getif = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getInterfaces","()[Ljava/lang/Class;");
                    PYJAVA_IGNORE_EXCEPTION(env);
                    if (getif){
                        jclass ifs = PYJAVA_ENVCALL(env,CallObjectMethod,klass,getif);
                        PYJAVA_IGNORE_EXCEPTION(env);
                        if (ifs){
                            for (jsize i = 0;i<PYJAVA_ENVCALL(env,GetArrayLength,ifs);i++){
                                PYJAVA_IGNORE_EXCEPTION(env);
                                jclass ifc = PYJAVA_ENVCALL(env,GetObjectArrayElement,ifs,i);
                                PYJAVA_IGNORE_EXCEPTION(env);
                                PyList_Append(lbases,(PyObject*)pyjava_classAsType(env,ifc));
                                PYJAVA_ENVCALL(env,DeleteLocalRef,ifc);
                            }
                            PYJAVA_ENVCALL(env,DeleteLocalRef,ifs);
                        }
                    }
                    PyObject * bases = PyList_AsTuple(lbases);
                    Py_DecRef(lbases);
                    ret->pto.tp_bases = bases;
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
                                            methdef->name = strcpy((char*)pyjava_malloc(sizeof(char)*(strlen("<init>")+1)),"<init>");
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
                                                        methdef->name = strcpy((char*)pyjava_malloc(sizeof(char)*(strlen(tmp)+1)),tmp);
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
                                            PyList_Append(ret->dir,str);
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
                                                        fielddef->name = strcpy((char*)pyjava_malloc(sizeof(char)*(strlen(tmp)+1)),tmp);
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
                                            PyList_Append(ret->dir,str);
                                            Py_DecRef(str);
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
            ret->pto.tp_dict = PyDict_New();
            {
                for (Py_ssize_t i = 0;i<PyTuple_Size(ret->dir);i++){
                    printf(PyUnicode_AsUTF8(PyTuple_GET_ITEM(ret->dir,i)));
                    PyDict_SetItem(ret->pto.tp_dict,PyTuple_GET_ITEM(ret->dir,i),Py_None);
                }
            }
            {
                PyObject * ctx = PyDict_New();
                const char * def =
                        "def d(obj):\n"
                        "    import cpyjava\n"
                        "    return cpyjava.symbols(obj)\n"
                        ;
                PyRun_String(def, Py_single_input, PyEval_GetGlobals(), ctx);
                PyObject * d = PyRun_String("d", Py_eval_input, PyEval_GetGlobals(), ctx);
                PyDict_SetItemString(ret->pto.tp_dict,"__dir__",d);
                Py_DecRef(ctx);
                Py_DecRef(d);
            }


        }

        if (PyType_Ready(&(ret->pto))){
            //TODO cleanup
            return NULL;
        }

        pyjava_typecache_register(env,ret);

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
