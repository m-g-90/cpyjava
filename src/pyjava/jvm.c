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
#include "pyjava/conversion.h"

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


static int pyjava_jnicheck =
        #if defined(QT_DEBUG) || defined(DEBUG)
            1
        #else
            0
        #endif
        ;

PYJAVA_DLLSPEC void pyjava_checkJNI(int enable){
    pyjava_jnicheck = enable;
}

PYJAVA_DLLSPEC int pyjava_initJVM() {

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
            JavaVMOption options[50];
            int optioncount = 0;
            if (pyjava_jnicheck){
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-Xcheck:jni";
                options[optioncount+1].extraInfo = NULL;
                options[optioncount+1].optionString = "-Xcheck:jni:pedantic";
                options[optioncount+2].extraInfo = NULL;
                options[optioncount+2].optionString = "-Xcheck:jni:trace";
                optioncount += 3;
                if (0){
                    options[optioncount].extraInfo = NULL;
                    options[optioncount].optionString = "-verbose";
                    optioncount += 1;
                }
            }
            //tuning collector
            {
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-XX:+UseG1GC";
                optioncount += 1;
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-XXfullCompaction";
                optioncount += 1;
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-XgcPrio:throughput";
                optioncount += 1;
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-XX:+UseStringCache";
                optioncount += 1;
                options[optioncount].extraInfo = NULL;
                options[optioncount].optionString = "-XX:+UseFastAccessorMethods";
                optioncount += 1;




            }

            initArgs.options = options;
            initArgs.nOptions = optioncount;
            jint err = (*createJavaVM)(&vm, (void**)&env, &initArgs);
            if (err != JNI_OK){

            } else {
                PYJAVA_ENVCALL(vm,DetachCurrentThread);
                pyjava_setJVM(vm);
                return 1;
            }
        }
    }

    return 0;
}

PYJAVA_DLLSPEC JavaVM * pyjava_getJVM(){

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
                    pyjava_setJVM(vm);
            }
        }

        if (!pyjava_jvmptr){
            pyjava_initJVM();
        }

    }

    return pyjava_jvmptr;
}
PYJAVA_DLLSPEC void pyjava_setJVM(JavaVM * jvm){
    pyjava_jvmptr = jvm;
    {
        PYJAVA_START_JAVA(env);
        pyjava_conversion_forceInit(env);
        PYJAVA_END_JAVA(env);
    }
}

static thread_local JNIEnv * _pyjava_enter_exit_env = NULL;
static thread_local int _pyjava_enter_exit_env_borrowed = 0;
static thread_local unsigned _pyjava_enter_exit_count = 0;
PYJAVA_DLLSPEC void pyjava_enter(){
    if (_pyjava_enter_exit_count==0){
        JNIEnv * env = NULL;
        int bor = 0;
        _pyjava_start_java(&env,&bor);
        _pyjava_enter_exit_env = env;
        _pyjava_enter_exit_env_borrowed=bor;
    }
    _pyjava_enter_exit_count++;
}

PYJAVA_DLLSPEC void pyjava_exit(){
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
PYJAVA_DLLSPEC void _pyjava_start_java(JNIEnv ** env, int * borrowed){
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
            PYJAVA_ENVCALL(*env,EnsureLocalCapacity,100);
        } else {
            *borrowed = 1;
            *env = NULL;
        }
    }
}

PYJAVA_DLLSPEC void _pyjava_end_java(JNIEnv ** env, int * borrowed){
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

