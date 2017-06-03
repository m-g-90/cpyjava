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

#ifdef __cplusplus
extern "C"{
#endif

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

static jclass _pyjava_is_class_class = NULL;
int pyjava_is_class(JNIEnv * env,jobject obj){
    if (!_pyjava_is_class_class){
        jclass tmp = PYJAVA_ENVCALL(env,FindClass,"java/lang/Class");
        _pyjava_is_class_class = PYJAVA_ENVCALL(env,NewGlobalRef,tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,tmp);
    }
    if (!_pyjava_is_class_class){
        return 0;
    }
    return (int) PYJAVA_ENVCALL(env,IsInstanceOf,obj,_pyjava_is_class_class);
}

static jclass _pyjava_is_map_class = NULL;
int pyjava_is_map(JNIEnv * env,jobject obj){
    if (!_pyjava_is_map_class){
        jclass tmp = PYJAVA_ENVCALL(env,FindClass,"java/util/Map");
        _pyjava_is_map_class = PYJAVA_ENVCALL(env,NewGlobalRef,tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,tmp);
    }
    if (!_pyjava_is_map_class){
        return 0;
    }
    return (int) PYJAVA_ENVCALL(env,IsInstanceOf,obj,_pyjava_is_map_class);
}
static jclass _pyjava_is_list_class = NULL;
int pyjava_is_list(JNIEnv * env,jobject obj){
    if (!_pyjava_is_list_class){
        jclass tmp = PYJAVA_ENVCALL(env,FindClass,"java/util/List");
        _pyjava_is_list_class = PYJAVA_ENVCALL(env,NewGlobalRef,tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,tmp);
    }
    if (!_pyjava_is_list_class){
        return 0;
    }
    return (int) PYJAVA_ENVCALL(env,IsInstanceOf,obj,_pyjava_is_list_class);
}
static jclass _pyjava_is_set_class = NULL;
int pyjava_is_set(JNIEnv * env,jobject obj){
    if (!_pyjava_is_set_class){
        jclass tmp = PYJAVA_ENVCALL(env,FindClass,"java/util/Set");
        _pyjava_is_set_class = PYJAVA_ENVCALL(env,NewGlobalRef,tmp);
        PYJAVA_ENVCALL(env,DeleteLocalRef,tmp);
    }
    if (!_pyjava_is_set_class){
        return 0;
    }
    return (int) PYJAVA_ENVCALL(env,IsInstanceOf,obj,_pyjava_is_set_class);
}

void pyjava_method_cache_reset(JNIEnv *env){
    if (_pyjava_identityHash_system) {
        PYJAVA_ENVCALL(env,DeleteGlobalRef,_pyjava_identityHash_system);
    }
    _pyjava_identityHash_system = NULL;
    _pyjava_identityHash_system_identityHash = NULL;
    _pyjava_is_class_class = NULL;
}

#ifdef __cplusplus
}
#endif
