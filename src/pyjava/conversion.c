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

#include "pyjava/conversion.h"
#include "pyjava/type.h"
#include "pyjava/memory.h"
#include "pyjava/jvm.h"
#include "pyjava/method_cache.h"

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

static PyJavaObject * _pyjava_objectmap[PYJAVA_OBJECT_DEDUP_BUCKETS];

static PyObject * _pyjava_wrapObject(JNIEnv * env, jobject obj,PyJavaType * type,int disabledecorator){


    const unsigned hash = (unsigned) pyjava_method_cache_identityHash(env,obj);
    if (!disabledecorator){

        PyJavaObject * cur = _pyjava_objectmap[hash%PYJAVA_OBJECT_DEDUP_BUCKETS];
        while (cur){
            if (PYJAVA_ENVCALL(env,IsSameObject,obj,cur->obj)){
                Py_IncRef((PyObject*)cur);
                return (PyObject*)cur;
            }
            cur = (PyJavaObject*) cur->_dedupll;
        }
    }

    if (type) {       

        PyJavaObject * r = (PyJavaObject *) pyjava_malloc(sizeof(PyJavaObject));
        memset(r,0,sizeof(PyJavaObject));
        if (!disabledecorator){
            r->_dedupll = _pyjava_objectmap[hash%PYJAVA_OBJECT_DEDUP_BUCKETS];
            _pyjava_objectmap[hash%PYJAVA_OBJECT_DEDUP_BUCKETS] = r;
        } else {
            r->_dedupll = NULL;
        }
        r->ob_base.ob_refcnt = 1;
        r->obj = PYJAVA_ENVCALL(env,NewGlobalRef,obj);
        r->flags.entries.decoself = !!disabledecorator;
        PyObject_INIT(((PyObject*)r),((PyTypeObject*)type));

        return (PyObject*) r;

    }

    return NULL;

}

void _pyjava_removededupobject(JNIEnv * env,PyJavaObject * o){

    if (o->flags.entries.decoself){ // decoself object are not deduplicated
        return;
    }

    const unsigned hash = (unsigned) pyjava_method_cache_identityHash(env,o->obj);
    {

        PyJavaObject * cur = _pyjava_objectmap[hash%PYJAVA_OBJECT_DEDUP_BUCKETS];
        PyJavaObject * prev = NULL;
        while (cur){
            if (PYJAVA_ENVCALL(env,IsSameObject,o->obj,cur->obj)){
                if (prev){
                    prev->_dedupll = cur->_dedupll;
                } else {
                    _pyjava_objectmap[hash%PYJAVA_OBJECT_DEDUP_BUCKETS] = (PyJavaObject*) cur->_dedupll;
                }
                return;
            }
            prev = cur;
            cur = (PyJavaObject*) cur->_dedupll;
        }
    }
}

PyObject * pyjava_asWrappedObject(JNIEnv * env, PyObject * obj){

    if (!obj){
        PyErr_SetString(PyExc_TypeError,"Passed object cannot be returned a a wrapped java object.");
        return NULL;
    }

    if (pyjava_isJavaClass(obj->ob_type)){
        Py_IncRef(obj);
        return obj;
    }

    if (PyType_CheckExact(obj)){
        if (pyjava_isJavaClass((PyTypeObject*)obj)){
            jclass klass = ((PyJavaType*)obj)->klass;
            if (klass){
                jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,(jobject)klass);
                if (klassklass){
                    PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klassklass);
                    if (type){
                        PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                        Py_DecRef((PyObject*)type);
                        return _pyjava_wrapObject(env,(jobject)klass,type,0);
                    }
                    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
                }
            }
        }
    }

    PyErr_SetString(PyExc_TypeError,"Passed object cannot be returned a a wrapped java object.");
    return NULL;

}


PyObject * pyjava_asUnconvertedWrappedObject(JNIEnv * env,jobject obj){

    if (!obj){
        Py_RETURN_NONE;
    }

    jclass klass = PYJAVA_ENVCALL(env,GetObjectClass,obj);
    if (klass){
        PyTypeObject * type = pyjava_classAsType(env,klass);
        PYJAVA_ENVCALL(env,DeleteLocalRef,klass);
        if (type){
            return _pyjava_wrapObject(env,obj,(PyJavaType*)type,0);
        }
    }

    PyErr_SetString(PyExc_TypeError,"Passed object cannot be returned a a wrapped java object.");
    return NULL;

}

PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj){
    if (!obj){
        Py_RETURN_NONE;
    }

    if (pyjava_is_class(env,obj)){
        return (PyObject*) pyjava_classAsType(env,(jclass)obj);
    }

    jclass klass = PYJAVA_ENVCALL(env,GetObjectClass,obj);

    if (klass){
        // get type and check for converters

        PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klass);

        if (type) {

            PyObject * ret = _pyjava_convj2p(env,type,obj);

            if (!ret){
                ret = _pyjava_wrapObject(env,obj,type,0);
            }

            Py_DecRef((PyObject*)type);
            PYJAVA_ENVCALL(env,DeleteLocalRef,klass);

            return ret;

        }

        PYJAVA_ENVCALL(env,DeleteLocalRef,klass);

        PyErr_SetString(PyExc_TypeError,"Failed to convert java object to python (No type object)");
        return NULL;

    }

    PyErr_SetString(PyExc_TypeError,"Failed to convert java object to python (class not found)");
    return NULL;

}

PyObject * pyjava_asUndecoratedObject(JNIEnv * env,PyObject * obj){

    if (pyjava_isJavaClass(obj->ob_type)){
        PyJavaObject * self = (PyJavaObject*) obj;
        if (self->flags.entries.decoself){
            Py_IncRef(obj);
            return obj;
        } else {
            return _pyjava_wrapObject(env,self->obj,(PyJavaType*)self->ob_base.ob_type,1);
        }
    } else {
        PyErr_SetString(PyExc_Exception,"Passed object is not a java object");
        return NULL;
    }

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
                ret->z = !!PyObject_IsTrue(obj);
                if (!PyErr_Occurred()){
                    return 1;
                } else {
                    PyErr_Clear();
                }
            }
            break;
        case 'B':
            if (PyLong_Check(obj)){
                int ol=0;
                PY_LONG_LONG val = PyLong_AsLongLongAndOverflow(obj,&ol);
                if (!ol && (val==(jbyte)val)){
                    ret->b = (jbyte)val;
                    return 1;
                }
            }
            break;
        case 'C':
            if (PyLong_CheckExact(obj)){
                int ol=0;
                PY_LONG_LONG val = PyLong_AsLongLongAndOverflow(obj,&ol);
                if (!ol && (val==(jchar)val)){
                    ret->c = (jchar)val;
                    return 1;
                }
            }
            if (PyUnicode_Check(obj)){
                if ( (PyUnicode_GET_LENGTH(obj)) == 1 ){
                    ret->c = (jchar) PyUnicode_ReadChar(obj,0);
                    return 1;
                }
            }

            break;
        case 'S':
            if (PyLong_CheckExact(obj)){
                int ol=0;
                PY_LONG_LONG val = PyLong_AsLongLongAndOverflow(obj,&ol);
                if (!ol && (val==(jshort)val)){
                    ret->s = (jshort)val;
                    return 1;
                }
            }
            break;
        case 'I':
            if (PyLong_CheckExact(obj)){
                int ol=0;
                PY_LONG_LONG val = PyLong_AsLongLongAndOverflow(obj,&ol);
                if (!ol && (val==(jint)val)){
                    ret->i = (jint)val;
                    return 1;
                }
            }
            break;
        case 'J':
            if (PyLong_CheckExact(obj)){
                int ol=0;
                PY_LONG_LONG val = PyLong_AsLongLongAndOverflow(obj,&ol);
                if (!ol && (val==(jlong)val)){
                    ret->j = (jlong)val;
                    return 1;
                }
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
                if (lret){
                    if (PYJAVA_ENVCALL(env,IsInstanceOf,lret,klass)){
                        ret->l = lret;
                        Py_DecRef((PyObject*)type);
                        return 1;
                    } else {
                        PYJAVA_ENVCALL(env,DeleteLocalRef,lret);
                    }
                }
            }
            Py_DecRef((PyObject*)type);
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

    if (!strcmp(type->pto.tp_name,"class java.lang.String")){
        pyjava_registerConversion(env,type->klass,str_j2p,str_p2j);
    }

}

static void simple_forceinit(JNIEnv * env, const char * name){
    jclass klass = PYJAVA_ENVCALL(env,FindClass,name);
    PYJAVA_IGNORE_EXCEPTION(env);
    if (klass){
        PyObject * obj = (PyObject*) pyjava_classAsType(env,klass);
        if (PyErr_Occurred()){
            PyErr_Clear();
        }
        if (obj){
            Py_DecRef(obj);
        }
        PYJAVA_ENVCALL(env,DeleteLocalRef,klass);
    }
}

void pyjava_conversion_forceInit(JNIEnv * env){

    simple_forceinit(env,"java/lang/String");
    simple_forceinit(env,"java/lang/Integer");
    simple_forceinit(env,"java/lang/Long");
    simple_forceinit(env,"java/lang/Char");
    simple_forceinit(env,"java/lang/Short");
    simple_forceinit(env,"java/lang/Byte");
    simple_forceinit(env,"java/lang/Boolean");
    simple_forceinit(env,"java/lang/Float");
    simple_forceinit(env,"java/lang/Double");
    simple_forceinit(env,"java/math/BigInteger");

}

static jmethodID _pyjava_throwable_tostring = NULL;
int pyjava_exception_java2python(JNIEnv * env){
    if (PYJAVA_ENVCALL(env,ExceptionCheck)){
        jthrowable ex = PYJAVA_ENVCALL(env,ExceptionOccurred);
        PYJAVA_ENVCALL(env,ExceptionClear);
        if (ex){
            if (!_pyjava_throwable_tostring){
                jclass throwclass = pyjava_throwable_class(env);
                if (throwclass){
                    _pyjava_throwable_tostring = PYJAVA_ENVCALL(env,GetMethodID,throwclass,"toString","()Ljava/lang/String;");
                }
            }
            if (_pyjava_throwable_tostring){
                jobject str = PYJAVA_ENVCALL(env,CallObjectMethod,ex,_pyjava_throwable_tostring);
                if (pyjava_exception_java2python(env)){
                    PyErr_Clear();
                } else {
                    const char * cstr = PYJAVA_ENVCALL(env,GetStringUTFChars,str,NULL);
                    if (cstr){
                        char * strcopy = pyjava_malloc(strlen(cstr)+sizeof(char));
                        strcpy(strcopy,cstr);
                        {
                            char * cur = strcopy;
                            while (*cur){
                                if (*cur == '\n'){
                                    *cur = ' ';
                                }
                                cur++;
                            }
                        }
                        PyErr_SetString(PyExc_Exception,(const char *)strcopy);
                        pyjava_free(strcopy);
                        PYJAVA_ENVCALL(env,ReleaseStringUTFChars,str,cstr);
                        PYJAVA_ENVCALL(env,DeleteLocalRef,str);
                        PYJAVA_ENVCALL(env,DeleteLocalRef,ex);
                        return 1;
                    }
                }
            }
        }
        PYJAVA_ENVCALL(env,Throw,ex);
        PYJAVA_ENVCALL(env,DeleteLocalRef,ex);
        PYJAVA_ENVCALL(env,ExceptionDescribe);
        PYJAVA_ENVCALL(env,ExceptionClear);
        PyErr_SetString(PyExc_Exception,"A java exception has occured, but translation failed");
        return 1;
    } else {
        return 0;
    }
}


PYJAVA_DLLSPEC void pyjava_registerConversion(JNIEnv * env, jclass klass, pyjava_converter_j2p_t cj2p, pyjava_converter_p2j_t cp2j){

    PyJavaType * type = (PyJavaType*) pyjava_classAsType(env,klass);

    if (type){
        _pyjava_addJ2P(&(type->converter),cj2p);
        _pyjava_addP2J_rec(type,cp2j);
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


PYJAVA_DLLSPEC void pyjava_registerNativeConversion(char ntype,pyjava_native_converter_j2p_t fnc){
    if (!fnc)
        return;

    PYJAVA_RNC_ADD(L);

}

PyObject * pyjava_compile(const char * code, PyObject * locals, int eval){
    PyObject * globals = PyEval_GetGlobals();
    if (globals){
        Py_IncRef(globals);
    } else {
        PyObject * module = PyImport_ImportModule("builtins");
        if (module){
            globals = PyModule_GetDict(module);
            globals = PyDict_Copy(globals);
            Py_DecRef(module);
        }
    }
    if (!globals){
        if (PyErr_Occurred()){
            PyErr_Clear();
        }
        return NULL;
    }
    int owned_locals = 0;
    if (!locals){
        locals = PyDict_New();
        owned_locals = 1;
    }
    PyObject * ret = PyRun_String(code,eval?Py_eval_input:Py_file_input,globals,locals);
    if (owned_locals){
        Py_DecRef(locals);
    }
    Py_DecRef(globals);
    if (PyErr_Occurred()){
        PyErr_Clear();
    }
    return ret;
}




#ifdef __cplusplus
}
#endif
