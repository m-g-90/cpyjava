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

#include "pyjava/jvm.h"

#ifndef __cplusplus
#ifdef _MSC_VER
#define thread_local __declspec( thread )
#else
#define thread_local __thread
#endif
#endif

#ifdef PYJAVA_JVM_DLOPEN
        #include <dlfcn.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

static JavaVM * pyjava_jvmptr = NULL;
typedef jint (*createJavaVM_t)(JavaVM **, void **, void *) ;

int pyjava_initJVM() {

    if (!pyjava_jvmptr){
        jint (*createJavaVM)(JavaVM **, void **, void *) = NULL;

#ifdef PYJAVA_JVM_FORCELINK
        if (!createJavaVM){
            createJavaVM = &JNI_CreateJavaVM;
        }
#endif
#ifdef PYJAVA_JVM_DLOPEN
        if (!createJavaVM){
            void * handle = dlopen("jvm",RTLD_LOCAL);
            if (handle){
                createJavaVM = (createJavaVM_t) dlsym(handle,"JNI_CreateJavaVM");
            } else {
                printf("%s",dlerror());
            }
            if (!createJavaVM){
                createJavaVM = (createJavaVM_t) dlsym(NULL,"JNI_CreateJavaVM");
            }
        }
#endif

        if (createJavaVM){
            JNIEnv *env;
            JavaVM *vm;
            JavaVMInitArgs initArgs;
            initArgs.version = JNI_VERSION_1_8;
            initArgs.ignoreUnrecognized = JNI_TRUE;
            JavaVMOption options[10];
            {
                options[0].extraInfo = NULL;
                options[0].optionString = "-Xcheck:jni";
            }
            int optioncount = 1;
            initArgs.options = options;
            initArgs.nOptions = optioncount;
            jint err = (*createJavaVM)(&vm, (void**)&env, &initArgs);
            if (err != JNI_OK){

            } else {
                PYJAVA_ENVCALL(vm,DetachCurrentThread);
                pyjava_jvmptr = vm;
                return 1;
            }
        }
    }

    return 0;
}

JavaVM * pyjava_getJVM(){

    // try getting existing jvms
    if (!pyjava_jvmptr){

        jint (*getCreatedJavaVMs)(JavaVM **, jsize, jsize *) = NULL;

#ifdef PYJAVA_JVM_FORCELINK
        if (!getCreatedJavaVMs){
            getCreatedJavaVMs = &JNI_GetCreatedJavaVMs;
        }
#endif
#ifdef PYJAVA_JVM_DLOPEN
        if (!getCreatedJavaVMs){
            typedef jint JNICALL (*getCreatedJavaVMs_t)(JavaVM **, jsize, jsize *);
            getCreatedJavaVMs = (getCreatedJavaVMs_t) dlsym(NULL,"JNI_GetCreatedJavaVMs");
        }
#endif

        if (getCreatedJavaVMs){
            JavaVM *vm;
            jsize vmcount;
            if (JNI_OK == getCreatedJavaVMs(&vm,1,&vmcount)){
                if (vmcount>0)
                    pyjava_jvmptr = vm;
            }
        }

        if (!pyjava_jvmptr){
            pyjava_initJVM();
        }

    }

    return pyjava_jvmptr;
}
void pyjava_setJVM(JavaVM * jvm){
    pyjava_jvmptr = jvm;
}

static thread_local JNIEnv * _pyjava_enter_exit_env = NULL;
static thread_local int _pyjava_enter_exit_env_borrowed = 0;
static thread_local unsigned _pyjava_enter_exit_count = 0;
void pyjava_enter(){
    if (_pyjava_enter_exit_count==0){
        JNIEnv * env = NULL;
        int bor = 0;
        _pyjava_start_java(&env,&bor);
        _pyjava_enter_exit_env = env;
        _pyjava_enter_exit_env_borrowed=bor;
    }
    _pyjava_enter_exit_count++;
}

void pyjava_exit(){
    _pyjava_enter_exit_count--;
    if (_pyjava_enter_exit_count==0){
        JNIEnv * env = _pyjava_enter_exit_env;
        int bor = _pyjava_enter_exit_env_borrowed;
        _pyjava_end_java(&env,&bor);
        _pyjava_enter_exit_env = env;
        _pyjava_enter_exit_env_borrowed=bor;
    }
}

static thread_local JNIEnv * _pyjava_env = NULL;
void _pyjava_start_java(JNIEnv ** env, int * borrowed){
    if (_pyjava_env){
        *borrowed = 1;
        *env = _pyjava_env;
    } else {
        JavaVM * vm = pyjava_getJVM();
        if (vm){
            if (PYJAVA_ENVCALL(vm,GetEnv,(void**)env,JNI_VERSION_1_8) == JNI_OK){
                _pyjava_env = *env;
                *borrowed = 2;
            } else {
                PYJAVA_ENVCALL(vm,AttachCurrentThread,(void **)env, NULL);
                _pyjava_env = *env;
                *borrowed = 0;
            }
        } else {
            *borrowed = 1;
            *env = NULL;
        }
    }
}

void _pyjava_end_java(JNIEnv ** env, int * borrowed){
    *env=NULL;
    if (*borrowed){
        if (*borrowed == 2){
            *borrowed = 1;
            _pyjava_env = 0;
        }
    } else {
        JavaVM * vm = pyjava_getJVM();
        PYJAVA_ENVCALL(vm,DetachCurrentThread);
        _pyjava_env = NULL;
    }
}


#ifdef __cplusplus
}
#endif

