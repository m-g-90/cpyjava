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


#include "pyjava/method_cache.h"
#include "pyjava/pyjava.h"
#include "pyjava/jvm.h"
#include "pyjava/type.h"

#ifdef __cplusplus
extern "C"{
#endif

#define PYJAVA_METHOD_CACHE_CLASS(CLASSNAME,SHORTNAME) \
    static jclass _pyjava_##SHORTNAME##_class = NULL; \
    jclass pyjava_##SHORTNAME##_class(JNIEnv * env){ \
        if (!_pyjava_##SHORTNAME##_class){ \
            jclass tmp = PYJAVA_ENVCALL(env,FindClass,CLASSNAME); \
            _pyjava_##SHORTNAME##_class = PYJAVA_ENVCALL(env,NewGlobalRef,tmp); \
            PYJAVA_ENVCALL(env,DeleteLocalRef,tmp); \
        } \
        if (!_pyjava_##SHORTNAME##_class){ \
            /*TODO: log error */ \
            return 0; \
        } \
        return _pyjava_##SHORTNAME##_class; \
    }

PYJAVA_METHOD_CACHE_CLASS("java/util/Iterator",iterator)
PYJAVA_METHOD_CACHE_CLASS("java/lang/Iterable",iterable)
PYJAVA_METHOD_CACHE_CLASS("java/lang/Object",object)
PYJAVA_METHOD_CACHE_CLASS("java/util/Set",set)
PYJAVA_METHOD_CACHE_CLASS("java/util/List",list)
PYJAVA_METHOD_CACHE_CLASS("java/util/Map",map)
PYJAVA_METHOD_CACHE_CLASS("java/lang/Class",class)
PYJAVA_METHOD_CACHE_CLASS("java/lang/Comparable",compareable)
PYJAVA_METHOD_CACHE_CLASS("java/lang/Throwable",throwable)

#define PYJAVA_METHOD_CACHE_METHOD(SHORTNAME,METHODNAME,SIGNATURE)\
    static jmethodID _pyjava_##SHORTNAME##_##METHODNAME##_method = NULL; \
    jmethodID pyjava_##SHORTNAME##_##METHODNAME##_method(JNIEnv * env){ \
        if (!_pyjava_##SHORTNAME##_##METHODNAME##_method){\
            jclass klass = pyjava_##SHORTNAME##_class(env);\
            if (klass) { \
                _pyjava_##SHORTNAME##_##METHODNAME##_method = PYJAVA_ENVCALL(env,GetMethodID,klass, PYJAVA_TOSTRING(METHODNAME),SIGNATURE); \
            }\
        }\
        return _pyjava_##SHORTNAME##_##METHODNAME##_method;\
    }

PYJAVA_METHOD_CACHE_METHOD(compareable,compareTo,"(Ljava/lang/Object;)I")

static jclass _pyjava_identityHash_system = NULL;
static jmethodID _pyjava_identityHash_system_identityHash = NULL;
jint pyjava_method_cache_identityHash(JNIEnv * env,jobject obj){

    if (!_pyjava_identityHash_system){
        jclass system = PYJAVA_ENVCALL(env,FindClass, "java/lang/System");
        PYJAVA_IGNORE_EXCEPTION(env);
        if (system){
            _pyjava_identityHash_system = PYJAVA_ENVCALL(env,NewGlobalRef,system);
        }
        PYJAVA_ENVCALL(env,DeleteLocalRef,system);
    }

    if (!_pyjava_identityHash_system){
        return 0;
    }

    if (!_pyjava_identityHash_system_identityHash){
        _pyjava_identityHash_system_identityHash = PYJAVA_ENVCALL(env,GetStaticMethodID,_pyjava_identityHash_system,"identityHashCode","(Ljava/lang/Object;)I");
        PYJAVA_IGNORE_EXCEPTION(env);
    }

    if (!_pyjava_identityHash_system_identityHash){
        return 0;
    }

    jint ret = PYJAVA_ENVCALL(env,CallStaticIntMethod,_pyjava_identityHash_system,_pyjava_identityHash_system_identityHash,obj);
    PYJAVA_IGNORE_EXCEPTION(env);
    return ret;

}

int pyjava_is_class(JNIEnv * env,jobject obj){
    if (!pyjava_class_class(env)){
        return 0;
    }
    return (int) PYJAVA_ENVCALL(env,IsInstanceOf,obj,_pyjava_class_class);
}

static jmethodID _pyjava_is_array_class = NULL;
int pyjava_is_arrayclass(JNIEnv * env,jclass obj){
    if (!pyjava_class_class(env)){
        return 0;
    }
    if (!_pyjava_is_array_class){
        _pyjava_is_array_class = PYJAVA_ENVCALL(env,GetMethodID,_pyjava_class_class,"isArray","()Z");
    }
    if (!_pyjava_is_array_class){
        return 0;
    }

    jboolean isarray = PYJAVA_ENVCALL(env,CallBooleanMethod,obj, _pyjava_is_array_class);

    if (pyjava_exception_java2python(env)){
        PyErr_Clear();
        isarray = 0;
    }

    return isarray?1:0;

}

static jmethodID _pyjava_array_sub_class = NULL;
/**
 * @brief pyjava_get_array_sub_Type
 * @param env
 * @return new global reference to the array type
 */
jclass pyjava_get_array_sub_Type(JNIEnv * env,jclass klass) {
    if (!pyjava_is_arrayclass(env,klass)){
        return (char)0;
    }
    if (!_pyjava_array_sub_class){
        _pyjava_array_sub_class = PYJAVA_ENVCALL(env,GetMethodID,_pyjava_class_class,"getComponentType","()Ljava/lang/Class;");
        PYJAVA_ENVCALL(env,ExceptionDescribe);
    }
    if (!_pyjava_array_sub_class){
        return (char)0;
    }
    jclass subklass = PYJAVA_ENVCALL(env,CallObjectMethod,klass,_pyjava_array_sub_class);

    if (pyjava_exception_java2python(env)){
        PyErr_Clear();
        subklass = NULL;
    }

    if (!subklass){
        return subklass;
    }

    jclass ret = PYJAVA_ENVCALL(env,NewGlobalRef,subklass);
    PYJAVA_ENVCALL(env,DeleteLocalRef,subklass);
    return ret;
}


static jmethodID _pyjava_object_equal = NULL;
int pyjava_object_equal(JNIEnv * env,jobject o1,jobject o2) {
    if (!_pyjava_object_class){
        pyjava_object_class(env);
    }
    if (!_pyjava_object_class){
        return -1;
    }
    if (!_pyjava_object_equal){
        _pyjava_object_equal = PYJAVA_ENVCALL(env,GetMethodID,_pyjava_object_class,"equals","(Ljava/lang/Object;)Z");
    }
    if (!_pyjava_object_equal){
        return -1;
    }
    return !! PYJAVA_ENVCALL(env,CallBooleanMethod,o1,_pyjava_object_equal,o2);
}

void pyjava_method_cache_reset(JNIEnv *env){
    if (_pyjava_identityHash_system) {
        PYJAVA_ENVCALL(env,DeleteGlobalRef,_pyjava_identityHash_system);
    }
    _pyjava_identityHash_system = NULL;
    _pyjava_identityHash_system_identityHash = NULL;
    _pyjava_class_class = NULL;
}

#ifdef __cplusplus
}
#endif
