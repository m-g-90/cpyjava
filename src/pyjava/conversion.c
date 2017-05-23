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

#ifdef __cplusplus
extern "C"{
#endif

static const char * _pyjava_class_getName(JNIEnv * env,jclass klass){
    jclass klassklass = PYJAVA_ENVCALL(env,GetObjectClass,klass);
    jmethodID toString = PYJAVA_ENVCALL(env,GetMethodID,klassklass,"getName","()Ljava/lang/String;");
    jstring jname = PYJAVA_ENVCALL(env,CallObjectMethod,klass,toString);
    const char  *tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,jname, 0);
    const char * ret = strcpy((char*)pyjava_malloc(sizeof(char)*strlen(tmp)+1),tmp);
    PYJAVA_ENVCALL(env,ReleaseStringUTFChars, jname, tmp);
    PYJAVA_ENVCALL(env,DeleteLocalRef,jname);
    PYJAVA_ENVCALL(env,DeleteLocalRef,klassklass);
    return ret;
}

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
    if (ntype){
        switch (ntype){
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
        }
    }
    const char * name = _pyjava_class_getName(env,klass);
    if (PyUnicode_CheckExact(obj) && !strcmp(name,"java.lang.String")){
        pyjava_free((char *)name);
        ret->l = PYJAVA_ENVCALL(env,NewStringUTF,PyUnicode_AsUTF8(obj));
        if (PYJAVA_ENVCALL(env,IsInstanceOf,ret->l,klass)){
            return 1;
        } else {
            PYJAVA_ENVCALL(env,DeleteLocalRef,ret->l);
            return 0;
        }
    }
    pyjava_free((char *)name);
    return 0;
}

PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj){
    if (!obj){
        Py_RETURN_NONE;
    }
    jclass klass = PYJAVA_ENVCALL(env,GetObjectClass,obj);
    if (klass){
        const char * name = _pyjava_class_getName(env,klass);
        if (name){
            if (!strcmp(name,"java.lang.String")){
                const char * tmp = PYJAVA_ENVCALL(env,GetStringUTFChars,obj, 0);
                PyObject * ret = PyUnicode_FromString(tmp);
                PYJAVA_ENVCALL(env,ReleaseStringUTFChars, obj, tmp);
                PYJAVA_ENVCALL(env,DeleteLocalRef,klass);
                pyjava_free((void*)name);
                return ret;
            }
        }

        //if no conversion, wrap object
        PyTypeObject * type = pyjava_classAsType(env,klass);
        PyJavaObject * ret = PyObject_New(PyJavaObject, type);
        Py_DecRef((PyObject*)type);
        ret->obj = PYJAVA_ENVCALL(env,NewGlobalRef,obj);
        pyjava_free((void*)name);
        return (PyObject*)ret;
    }
    Py_RETURN_NONE;
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


#ifdef __cplusplus
}
#endif
