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

#include "pyjava/conversion.h"
#include "pyjava/type.h"
#include "pyjava/memory.h"
#include "pyjava/jvm.h"

#ifdef __cplusplus
extern "C"{
#endif

pyjava_converter_j2p_t _pyjava_getJ2P(PyJavaConverter * conv,int index){
    if (index < 0){
        return NULL;
    }
    if (index > conv->convcountj2p){
        return NULL;
    }
    if (index == 0){
        return conv->convj2p0;
    } else {
        return conv->convj2p1u[index-1];
    }
}
pyjava_converter_p2j_t _pyjava_getP2J(PyJavaConverter * conv,int index){
    if (index < 0){
        return NULL;
    }
    if (index > conv->convcountp2j){
        return NULL;
    }
    if (index == 0){
        return conv->convp2j0;
    } else {
        return conv->convp2j1u[index-1];
    }
}
void _pyjava_addJ2P(PyJavaConverter * conv,pyjava_converter_j2p_t fnc){
    if (!fnc)
        return;
    if (conv->convcountj2p==0){
        conv->convj2p0 = fnc;
    } else {
        pyjava_converter_j2p_t * tmp = (pyjava_converter_j2p_t *)pyjava_malloc(sizeof(pyjava_converter_j2p_t)*conv->convcountj2p);
        if (conv->convcountj2p>1){
            memcpy(tmp,conv->convj2p1u,sizeof(pyjava_converter_j2p_t)*(conv->convcountj2p-1));
        }
        tmp[conv->convcountj2p-1] = fnc;
        pyjava_free(conv->convj2p1u);
        conv->convj2p1u = tmp;
    }
    conv->convcountj2p++;
}
void _pyjava_addP2J(PyJavaConverter * conv,pyjava_converter_p2j_t fnc){
    if (!fnc)
        return;
    if (conv->convcountp2j==0){
        conv->convp2j0 = fnc;
    } else {
        pyjava_converter_p2j_t * tmp = (pyjava_converter_p2j_t *)pyjava_malloc(sizeof(pyjava_converter_p2j_t)*conv->convcountp2j);
        if (conv->convcountp2j>1){
            memcpy(tmp,conv->convp2j1u,sizeof(pyjava_converter_p2j_t)*(conv->convcountp2j-1));
        }
        tmp[conv->convcountp2j-1] = fnc;
        pyjava_free(conv->convp2j1u);
        conv->convp2j1u = tmp;
    }
    conv->convcountp2j++;
}
void _pyjava_addP2J_rec(PyJavaType * type,pyjava_converter_p2j_t fnc){

    if (!pyjava_isJavaClass((PyTypeObject*)type)){
        return;
    }

    _pyjava_addP2J(&(type->converter),fnc);

    if (type->pto.tp_bases && (PyTuple_CheckExact(type->pto.tp_bases))){
        for (Py_ssize_t i = 0;i<PyTuple_Size(type->pto.tp_bases);i++){
            _pyjava_addP2J_rec((PyJavaType*)PyTuple_GET_ITEM(type->pto.tp_bases,i),fnc);
        }
    }

}

static PyObject * _pyjava_convj2p(JNIEnv * env,PyJavaType * type, jobject obj){
    if (!pyjava_isJavaClass((PyTypeObject*)type)){
        return NULL;
    }
    for (int i = 0;i<type->converter.convcountj2p;i++){
        PyObject * ret = _pyjava_getJ2P(&(type->converter),i)(env,type->klass,obj);
        if (PyErr_Occurred()){
            PyErr_Clear();
            if (ret){
                Py_DecRef(ret);
                ret = NULL;
            }
        }
        if (ret){
            return ret;
        }
    }
    if (type->pto.tp_bases && (PyTuple_CheckExact(type->pto.tp_bases))){
        for (Py_ssize_t i = 0;i<PyTuple_Size(type->pto.tp_bases);i++){
            PyObject * ret = _pyjava_convj2p(env,(PyJavaType*)PyTuple_GET_ITEM(type->pto.tp_bases,i),obj);
            if (ret){
                return ret;
            }
        }
    }


    return NULL;
}

PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj){
    if (!obj){
        Py_RETURN_NONE;
    }
    jclass klass = PYJAVA_ENVCALL(env,GetObjectClass,obj);


    if (klass){
        // get type and check for converters

        PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klass);

        if (type) {

            PyObject * ret = _pyjava_convj2p(env,type,obj);

            if (!ret){
                PyJavaObject * r = (PyJavaObject *) pyjava_malloc(sizeof(PyJavaObject));
                r->ob_base.ob_refcnt = 1;
                r->ob_base.ob_type = &(type->pto);
                r->obj = PYJAVA_ENVCALL(env,NewGlobalRef,obj);
                ret = (PyObject*) r;
            } else {
                Py_DecRef((PyObject*)type);
            }

            return ret;

        }

        PyErr_SetString(PyExc_TypeError,"Failed to convert java object to python (No type object)");
        return NULL;

    }

    PyErr_SetString(PyExc_TypeError,"Failed to convert java object to python (class not found)");
    return NULL;

}

#define PYJAVA_RNC_DEF(T) \
    static pyjava_native_converter_j2p_t * native_converters_##T = NULL;\
    static int native_converters_##T##_count = 0

PYJAVA_RNC_DEF(L);

int pyjava_asJObject(JNIEnv * env, PyObject * obj,jclass klass,char ntype, jvalue * ret){
    if (!obj || obj==Py_None){
        memset(ret,0,sizeof(jvalue));
        return 1;
    }
    if (pyjava_isJavaClass(obj->ob_type)){
        //TODO maybe java to java conversions make sense in some cases
        if (PYJAVA_ENVCALL(env,IsInstanceOf,((PyJavaObject*)obj)->obj ,klass)){
            ret->l = PYJAVA_ENVCALL(env,NewLocalRef,((PyJavaObject*)obj)->obj);
            return 1;
        }
    }
    if (PyType_CheckExact(obj) && pyjava_isJavaClass((PyTypeObject*)obj)){
        //TODO maybe java to java conversions make sense in some cases
        if (PYJAVA_ENVCALL(env,IsInstanceOf,((PyJavaType*)obj)->klass ,klass)){
            ret->l = PYJAVA_ENVCALL(env,NewLocalRef,((PyJavaType*)obj)->klass);
            return 1;
        }
    }
    int isObject = 0;
    if (ntype){
        switch (ntype){
        case 'Z':
            if (PyBool_Check(obj)){
                ret->z = PyObject_IsTrue(obj);
                if (!PyErr_Occurred()){
                    return 1;
                } else {
                    PyErr_Clear();
                }
            }
            break;
        case 'B':
            if (PyLong_Check(obj)){
                ret->b = (jbyte) PyLong_AsLongLong(obj);
                return 1;
            }
            break;
        case 'C':
            if (PyLong_CheckExact(obj)){
                ret->c = (jchar) PyLong_AsLongLong(obj);
                return 1;
            }
            break;
        case 'S':
            if (PyLong_CheckExact(obj)){
                ret->s = (jshort) PyLong_AsLongLong(obj);
                return 1;
            }
            break;
        case 'I':
            if (PyLong_CheckExact(obj)){
                ret->i = (jint) PyLong_AsLongLong(obj);
                return 1;
            }
            break;
        case 'J':
            if (PyLong_CheckExact(obj)){
                ret->j = PyLong_AsLongLong(obj);
                return 1;
            }
            break;
        case 'F':
            if (PyFloat_CheckExact(obj)){
                ret->f = (jfloat) PyFloat_AsDouble(obj);
                return 1;
            }
            break;
        case 'D':
            if (PyFloat_CheckExact(obj)){
                ret->d = (jdouble) PyFloat_AsDouble(obj);
                return 1;
            }
            break;
        case 'L':
        case '[':
            isObject = 1;
            break;
        default:
            printf("Unhandled type");
            return 0;
            break;
        }
    }

    if (isObject){
        PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klass);
        if (type){
            for (int i = 0;i<type->converter.convcountp2j;i++){
                jobject lret = _pyjava_getP2J(&(type->converter),i)(env,type->klass,obj);
                Py_DecRef((PyObject*)type);
                if (lret){
                    if (PYJAVA_ENVCALL(env,IsInstanceOf,lret,klass)){
                        ret->l = lret;
                        return 1;
                    } else {
                        PYJAVA_ENVCALL(env,DeleteLocalRef,lret);
                    }
                }
            }
        }
    }

    return 0;
}


static PyObject * str_j2p(JNIEnv* env,jclass klass,jobject obj){
    (void)klass;
    const char * tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,obj, 0);
    PyObject * ret = PyUnicode_FromString(tmp);
    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, obj, tmp);
    return ret;
}

static jobject str_p2j(JNIEnv * env,jclass klass,PyObject * obj){
    (void)klass;
    if (PyUnicode_CheckExact(obj)){
        return PYJAVA_ENVCALL(env,NewStringUTF,PyUnicode_AsUTF8(obj));
    }
    return NULL;
}

void pyjava_conversion_initType(JNIEnv * env,PyJavaType * type){

    if (!strcmp(type->pto.tp_name,"java.lang.String")){
        pyjava_registerConversion(env,type->klass,str_j2p,str_p2j);
    }

}


int pyjava_exception_java2python(JNIEnv * env){
    if (PYJAVA_ENVCALL(env,ExceptionCheck)){
        PYJAVA_ENVCALL(env,ExceptionClear);
        PyErr_SetString(PyExc_NotImplementedError,"A java exception has occured, but translation is not yet supported");
        return 1;
    } else {
        return 0;
    }
}


void pyjava_registerConversion(JNIEnv * env, jclass klass, pyjava_converter_j2p_t cj2p, pyjava_converter_p2j_t cp2j){

    PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klass);

    if (type){
        _pyjava_addJ2P(&(type->converter),cj2p);
        _pyjava_addP2J(&(type->converter),cp2j);
    }

}


#define PYJAVA_RNC_ADD(T) \
    if (ntype == ((#T)[0])) {\
        pyjava_native_converter_j2p_t * tmp = (pyjava_native_converter_j2p_t *) pyjava_malloc( sizeof(pyjava_native_converter_j2p_t) *(native_converters_##T##_count + 1));\
        if (native_converters_##T##_count>0){\
            memcpy(tmp,native_converters_##T,sizeof(pyjava_native_converter_j2p_t)*native_converters_##T##_count);\
        }\
        tmp[native_converters_##T##_count] = fnc;\
        if (native_converters_##T##_count>0){\
            pyjava_free(native_converters_##T);\
        }\
        native_converters_##T = tmp;\
        native_converters_##T##_count++;\
        return;\
    }


void pyjava_registerNativeConversion(char ntype,pyjava_native_converter_j2p_t fnc){
    if (!fnc)
        return;

    PYJAVA_RNC_ADD(L);

}


#ifdef __cplusplus
}
#endif
