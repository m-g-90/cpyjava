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


#include "pyjava/method_cache.h"
#include "pyjava/pyjava.h"

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

void pyjava_method_cache_reset(JNIEnv *env){
    if (_pyjava_identityHash_system) {
        PYJAVA_ENVCALL(env,DeleteGlobalRef,_pyjava_identityHash_system);
    }
    _pyjava_identityHash_system = NULL;
    _pyjava_identityHash_system_identityHash = NULL;
}

#ifdef __cplusplus
}
#endif
