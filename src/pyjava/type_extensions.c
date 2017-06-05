
#include "pyjava/type_extensions.h"
#include "pyjava/method_cache.h"
#include "pyjava/jvm.h"

#ifdef __cplusplus
extern "C"{
#endif


static Py_ssize_t java_util_Map_Length(PyJavaObject *o){
    PYJAVA_START_JAVA(env);
    jclass klass = pyjava_is_map_class(env);
    if (klass){
        jmethodID mid = PYJAVA_ENVCALL(env,GetMethodID,klass,"size","()I");
        if (mid){
            if (o && o->obj){
                Py_ssize_t ret = (Py_ssize_t) PYJAVA_ENVCALL(env,CallIntMethod,o->obj,mid);
                if (PYJAVA_ENVCALL(env,ExceptionCheck)) {
                    PYJAVA_ENVCALL(env,ExceptionClear);
                    ret = 0;
                }
                return ret;
            }
        }
    }
    PYJAVA_END_JAVA(env);
    return 0;
}

static PyObject * member_call0(PyObject * obj,const char * name){
    PyObject * get = PyObject_GetAttrString(obj,name);
    if (PyErr_Occurred()){
        return NULL;
    }
    PyObject * args = PyTuple_New(0);
    PyObject * ret = PyObject_Call(get,args,NULL);
    Py_DecRef(args);
    Py_DecRef(get);
    return ret;
}
static PyObject * member_call1(PyObject * obj,const char * name,PyObject * arg0){
    PyObject * get = PyObject_GetAttrString(obj,name);
    if (PyErr_Occurred()){
        return NULL;
    }
    PyObject * args = PyTuple_New(1);
    Py_IncRef(arg0);
    PyTuple_SetItem(args,0,arg0);
    PyObject * ret = PyObject_Call(get,args,NULL);
    Py_DecRef(args);
    Py_DecRef(get);
    return ret;
}
static PyObject * member_call2(PyObject * obj,const char * name,PyObject * arg0,PyObject * arg1){
    PyObject * get = PyObject_GetAttrString(obj,name);
    if (PyErr_Occurred()){
        return NULL;
    }
    PyObject * args = PyTuple_New(2);
    Py_IncRef(arg0);
    Py_IncRef(arg1);
    PyTuple_SetItem(args,0,arg0);
    PyTuple_SetItem(args,1,arg1);
    PyObject * ret = PyObject_Call(get,args,NULL);
    Py_DecRef(args);
    Py_DecRef(get);
    return ret;
}

static PyObject * java_util_Map_Get(PyObject *o,PyObject * key){
    return member_call1((PyObject *)o,"get",key);
}
static int java_util_Map_Set(PyObject *o,PyObject * key,PyObject * val){

    if (val) {
        PyObject * ret = member_call2((PyObject *)o,"put",key,val);
        if (ret){
            Py_DecRef(ret);
        }
        if (PyErr_Occurred()){
            return -1;
        }
        return 0;
    } else {
        PyObject * ret = member_call1((PyObject *)o,"remove",key);
        if (ret){
            Py_DecRef(ret);
        }
        if (PyErr_Occurred()){
            return -1;
        }
        return 0;
    }
}

static PyObject * java_lang_Iterable_iterator(PyObject *o){

    return member_call0(o,"iterator");

}

static PyObject * java_util_Map_tp_iter(PyObject *o){

    PyObject * ret = member_call0(o,"keySet");

    if (ret){

        ret = java_lang_Iterable_iterator(ret);

    }

    return ret;

}

static PyObject * java_lang_Iterator_next(PyObject *o){

    PyObject * ret = member_call0(o,"next");

    if (PyErr_Occurred()){ //TODO handle the case where actually an error occured
        PyErr_Clear();
        return NULL;
    }

    return ret;

}

static PyObject * java_lang_Iterator_tp_iter(PyObject *o){

    Py_IncRef(o);

    return o;

}

static PyMappingMethods java_util_Map = {
    (lenfunc)&java_util_Map_Length,
    &java_util_Map_Get,
    &java_util_Map_Set
};

void pyjava_init_type_extensions(JNIEnv * env,PyJavaType * type){

    // some general interfaces. might be overriden later if special behaviour for more complex interfaces is needed
    if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_iterable_class(env))){
        type->pto.tp_iter = &java_lang_Iterable_iterator;
    }
    if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_iterator_class(env))){
        type->pto.tp_iter = &java_lang_Iterator_tp_iter;
        type->pto.tp_iternext = &java_lang_Iterator_next;
    }


    if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_is_map_class(env))){
        type->pto.tp_as_mapping = &java_util_Map;
        type->pto.tp_iter = &java_util_Map_tp_iter;
    } else if (0) { // check if this is a list

    }

}

#ifdef __cplusplus
}
#endif
