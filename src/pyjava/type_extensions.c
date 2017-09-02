
#include "pyjava/type_extensions.h"
#include "pyjava/method_cache.h"
#include "pyjava/jvm.h"

#ifdef __cplusplus
extern "C"{
#endif


static Py_ssize_t java_util_Map_Length(PyJavaObject *o){
    PYJAVA_START_JAVA(env);
    jclass klass = pyjava_map_class(env);
    if (klass){
        jmethodID mid = PYJAVA_ENVCALL(env,GetMethodID,klass,"size","()I");
        if (mid){
            if (o && o->obj){
                Py_ssize_t ret = (Py_ssize_t) PYJAVA_ENVCALL(env,CallIntMethod,o->obj,mid);
                if (PYJAVA_ENVCALL(env,ExceptionCheck)) {
                    PYJAVA_ENVCALL(env,ExceptionClear);
                    ret = 0;
                }
                PYJAVA_END_JAVA(env);
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

static Py_ssize_t java_util_List_Length(PyObject * o){
    PyObject * ret = member_call0((PyObject *)o,"size");
    if (ret){
        if (PyLong_CheckExact(ret)){
            Py_ssize_t l = PyLong_AsSsize_t(ret);
            Py_DecRef(ret);
            return l;
        } else {
            Py_DecRef(ret);
        }
    }
    return -1;
}

static PyObject * java_util_List_Concat(PyObject * o,PyObject * val){
    return member_call1((PyObject *)o,"addAll",val);
}

static int java_util_Set_contains(PyObject * o,PyObject * val){
    PyObject * ret = member_call1((PyObject *)o,"contains",val);
    if (!ret){
        return -1;
    }
    int result = ret==Py_True ? 1 : 0;
    Py_DecRef(ret);
    return result;
}

static PyObject * java_util_List_Item(PyObject *o,Py_ssize_t index){
    PyObject * val = PyLong_FromSsize_t(index);
    PyObject * ret = member_call1((PyObject *)o,"get",val);
    Py_DecRef(val);
    return ret;
}

static int java_util_List_Ass_Item(PyObject *o,Py_ssize_t index,PyObject * val){
    PyObject * i = PyLong_FromSsize_t(index);
    PyObject * ret = NULL;
    if (val){
        ret = member_call2((PyObject *)o,"set",i,val);
    } else {
        ret = member_call1((PyObject *)o,"remove",i);
    }
    Py_DecRef(i);
    if (ret)
        Py_DecRef(ret);
    return PyErr_Occurred()?-1:0;
}

static PyMappingMethods java_util_Map = {
    (lenfunc)&java_util_Map_Length,
    &java_util_Map_Get,
    &java_util_Map_Set
};

static PySequenceMethods java_util_List = {
    &java_util_List_Length,// lenfunc sq_length,
    &java_util_List_Concat,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_util_List_Item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_util_List_Ass_Item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};

static PySequenceMethods java_util_Set = {
    &java_util_List_Length,// lenfunc sq_length,
    &java_util_List_Concat,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    0,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    0,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    &java_util_Set_contains,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};

static PyObject * java_lang_compareable(PyObject * self, PyObject * other, int op){

    PyObject * ret = NULL;

    PYJAVA_START_JAVA(env);
    jmethodID meth = pyjava_compareable_compareTo_method(env);
    if (meth){
        jvalue jval;
        jval.l = NULL;
        if (pyjava_asJObject(env,other,pyjava_object_class(env),'L',&jval)){
            int result = PYJAVA_ENVCALL(env,CallIntMethodA,((PyJavaObject*)self)->obj,meth,&jval);
            if (!pyjava_exception_java2python(env)){
                switch (op) {
                case Py_EQ:
                    ret = PyBool_FromLong(result?0:1);
                    break;
                case Py_NE:
                    ret = PyBool_FromLong(result?1:0);
                    break;
                case Py_GT:
                    ret = PyBool_FromLong(result>0?1:0);
                    break;
                case Py_GE:
                    ret = PyBool_FromLong(result>=0?1:0);
                    break;
                case Py_LT:
                    ret = PyBool_FromLong(result<0?1:0);
                    break;
                case Py_LE:
                    ret = PyBool_FromLong(result<=0?1:0);
                    break;
                default:
                    break;
                }
            }
        }
    }
    PYJAVA_END_JAVA(env);

    if (!ret && !PyErr_Occurred()){
        PyErr_SetString(PyExc_Exception,"comparison failed");
    }

    return ret;

}

static Py_ssize_t java_array_length(PyObject * o){
    Py_ssize_t size = -1;
    PYJAVA_START_JAVA(env);
    size = PYJAVA_ENVCALL(env,GetArrayLength,((PyJavaObject*)o)->obj);
    PYJAVA_END_JAVA(env);
    return size;
}

static PyObject * java_object_array_item(PyObject * o,Py_ssize_t index){
    PyObject * ret = NULL;
    PYJAVA_START_JAVA(env);
    if (env){
        Py_ssize_t size = PYJAVA_ENVCALL(env,GetArrayLength,((PyJavaObject*)o)->obj);
        if (!pyjava_exception_java2python(env)){
            if (index < 0){
                if (size >= -index){
                    index = size-index;
                }
            }
            if (index < 0 || index >= size){
                PyErr_SetString(PyExc_IndexError,"Index out of range");
            } else {
                jobject obj = PYJAVA_ENVCALL(env,GetObjectArrayElement,((PyJavaObject*)o)->obj,(jsize) index);
                if (!pyjava_exception_java2python(env)){
                    ret = pyjava_asPyObject(env,obj);
                    PYJAVA_ENVCALL(env,DeleteLocalRef,obj);
                }
            }
        }
    }
    PYJAVA_END_JAVA(env);
    return ret;
}

static int java_object_array_ass_item(PyObject * o,Py_ssize_t index,PyObject * val){
    if (!val){
        PyErr_SetString(PyExc_Exception,"cannot change length of a java array");
        return -1;
    }
    PYJAVA_START_JAVA(env);
    if (env){
        Py_ssize_t size = PYJAVA_ENVCALL(env,GetArrayLength,((PyJavaObject*)o)->obj);
        if (!pyjava_exception_java2python(env)){
            if (index < 0){
                if (size >= -index){
                    index = size-index;
                }
            }
            if (index < 0 || index >= size){
                PyErr_SetString(PyExc_IndexError,"Index out of range");
            } else {
                jvalue jval;
                if (pyjava_asJObject(env,val,((PyJavaType*)((PyJavaObject*)o->ob_type))->arrayklass,((PyJavaType*)((PyJavaObject*)o->ob_type))->arrayntype,&jval)){
                    PYJAVA_ENVCALL(env,SetObjectArrayElement,((PyJavaObject*)o)->obj,(jsize) index,jval.l);
                    if (!pyjava_exception_java2python(env)){

                    }
                    PYJAVA_ENVCALL(env,DeleteLocalRef,jval.l);
                }
            }
        }
    }
    PYJAVA_END_JAVA(env);
    if (PyErr_Occurred()){
        return -1;
    } else {
        return 0;
    }
}

#define PYJAVA_ARRAY_INT_SEQUENCE(NAME,JTYPE,UJTYPE,MODE) \
    static PyObject * java_##NAME##_array_item(PyObject * o,Py_ssize_t index){\
    PyObject * ret = NULL;\
    PYJAVA_START_JAVA(env);\
    if (env){\
        Py_ssize_t size = PYJAVA_ENVCALL(env,GetArrayLength,((PyJavaObject*)o)->obj);\
        if (!pyjava_exception_java2python(env)){\
            if (index < 0){\
                if (size >= -index){\
                    index = size-index;\
                }\
            }\
            if (index < 0 || index >= size){\
                PyErr_SetString(PyExc_IndexError,"Index out of range");\
            } else {\
                JTYPE * obj = (JTYPE *) PYJAVA_ENVCALL(env,GetPrimitiveArrayCritical,((PyJavaObject*)o)->obj,NULL);\
                switch (MODE){ \
                    case 0: ret = PyLong_FromLongLong((long long)obj[index]); break; \
                    case 1: ret = PyFloat_FromDouble((double)obj[index]); break; \
                    case 2: ret = PyBool_FromLong((long)obj[index]); break; \
                    case 3: { \
                        wchar_t str[2]; \
                        str[0] = (wchar_t) obj[index]; \
                        str[1] = 0; \
                        ret = PyUnicode_FromWideChar(str,1); \
                    } \
                    break; \
                } \
                PYJAVA_ENVCALL(env,ReleasePrimitiveArrayCritical,((PyJavaObject*)o)->obj,(void*)obj,JNI_ABORT);\
            }\
        }\
    }\
    PYJAVA_END_JAVA(env);\
    return ret;\
    }\
    static int java_##NAME##_array_ass_item(PyObject * o,Py_ssize_t index,PyObject * val){\
        if (!val){\
            PyErr_SetString(PyExc_Exception,"cannot change length of a java array");\
            return -1;\
        }\
        switch (MODE){ \
            case 0: \
            {\
                if (!PyLong_CheckExact(val)){\
                    PyErr_SetString(PyExc_Exception,"wrong type for array");\
                    return -1;\
                }\
            }\
            break; \
            case 1: \
            {\
                if (!PyFloat_CheckExact(val)){\
                    PyErr_SetString(PyExc_Exception,"wrong type for array");\
                    return -1;\
                }\
            }\
            break; \
            case 2: \
            {\
                if (!PyBool_Check(val)){\
                    PyErr_SetString(PyExc_Exception,"wrong type for array");\
                    return -1;\
                }\
            }\
            break; \
            case 3: \
            {\
                if (!PyLong_CheckExact(val)){\
                    if (!PyUnicode_CheckExact(val) || PyUnicode_GetSize(val)!=1){\
                        PyErr_SetString(PyExc_Exception,"wrong type for array");\
                        return -1;\
                    }\
                }\
            }\
            break; \
        } \
        PYJAVA_START_JAVA(env);\
        if (env){\
            Py_ssize_t size = PYJAVA_ENVCALL(env,GetArrayLength,((PyJavaObject*)o)->obj);\
            if (!pyjava_exception_java2python(env)){\
                if (index < 0){\
                    if (size >= -index){\
                        index = size-index;\
                    }\
                }\
                if (index < 0 || index >= size){\
                    PyErr_SetString(PyExc_IndexError,"Index out of range");\
                } else {\
                    static_assert(sizeof(JTYPE)<=sizeof(long long),"long long smaller than" PYJAVA_TOSTRING(JTYPE)); \
                    JTYPE * obj = (JTYPE *) PYJAVA_ENVCALL(env,GetPrimitiveArrayCritical,((PyJavaObject*)o)->obj,NULL);\
                    switch (MODE){ \
                        case 0: obj[index] = (JTYPE) PyLong_AsLongLong(val); break; \
                        case 1: obj[index] = (JTYPE) PyFloat_AsDouble(val); break; \
                        case 2: obj[index] = (JTYPE) (val==Py_True?1:0); break; \
                        case 3: {\
                            if (PyLong_CheckExact(val)){ \
                                obj[index] = (JTYPE) PyLong_AsLongLong(val); \
                            } else { \
                                wchar_t x[2]; \
                                PyUnicode_AsWideChar(val,x,1); \
                                obj[index] = (JTYPE) x[0]; \
                            }\
                        }\
                        break; \
                    } \
                    PYJAVA_ENVCALL(env,ReleasePrimitiveArrayCritical,((PyJavaObject*)o)->obj,(void*)obj,0);\
                }\
            }\
        }\
        PYJAVA_END_JAVA(env);\
        if (PyErr_Occurred()){\
            return -1;\
        } else {\
            return 0;\
        }\
    }

PYJAVA_ARRAY_INT_SEQUENCE(boolean,jboolean,Boolean,2)
PYJAVA_ARRAY_INT_SEQUENCE(byte,jbyte,Byte,0)
PYJAVA_ARRAY_INT_SEQUENCE(char,jchar,Char,3)
PYJAVA_ARRAY_INT_SEQUENCE(short,jshort,Short,0)
PYJAVA_ARRAY_INT_SEQUENCE(int,jint,Int,0)
PYJAVA_ARRAY_INT_SEQUENCE(long,jlong,Long,0)
PYJAVA_ARRAY_INT_SEQUENCE(float,jfloat,Float,1)
PYJAVA_ARRAY_INT_SEQUENCE(double,jdouble,Double,1)

static PySequenceMethods java_bool_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_boolean_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_boolean_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_byte_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_byte_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_byte_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_char_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_char_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_char_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_short_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_short_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_short_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_int_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_int_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_int_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_long_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_long_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_long_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_float_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_float_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_float_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_double_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_double_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_double_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
};
static PySequenceMethods java_object_array = {
    &java_array_length,// lenfunc sq_length,
    0,// binaryfunc sq_concat,
    0,// ssizeargfunc sq_repeat,
    &java_object_array_item,// ssizeargfunc sq_item,
    0,// void *was_sq_slice,
    &java_object_array_ass_item,// ssizeobjargproc sq_ass_item,
    0,// void *was_sq_ass_slice,
    0,// objobjproc sq_contains,
    0,// binaryfunc sq_inplace_concat,
    0// ssizeargfunc sq_inplace_repeat
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
    if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_compareable_class(env))){
        type->tp_richcompare_impl = &java_lang_compareable;
    }


    if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_map_class(env))){
        type->pto.tp_as_mapping = &java_util_Map;
        type->pto.tp_iter = &java_util_Map_tp_iter;
    } else if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_list_class(env))) { // check if this is a list
        type->pto.tp_as_sequence = &java_util_List;
    } else if (PYJAVA_ENVCALL(env,IsAssignableFrom,type->klass,pyjava_set_class(env))) {
        type->pto.tp_as_sequence = &java_util_Set;
    } else if (type->arrayklass){
        switch (type->arrayntype){
        case 'Z':
            type->pto.tp_as_sequence = &java_bool_array; break;
        case 'B':
            type->pto.tp_as_sequence = &java_byte_array; break;
        case 'C':
            type->pto.tp_as_sequence = &java_char_array; break;
        case 'S':
            type->pto.tp_as_sequence = &java_short_array; break;
        case 'I':
            type->pto.tp_as_sequence = &java_int_array; break;
        case 'J':
            type->pto.tp_as_sequence = &java_long_array; break;
        case 'F':
            type->pto.tp_as_sequence = &java_float_array; break;
        case 'D':
            type->pto.tp_as_sequence = &java_double_array; break;
        case 'L':
        case '[':
            type->pto.tp_as_sequence = &java_object_array; break;
        }
    }

}

#ifdef __cplusplus
}
#endif
