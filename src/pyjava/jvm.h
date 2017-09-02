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

#ifndef PYJAVA_JVM_H
#define PYJAVA_JVM_H

#include "pyjava/config.h"

#ifdef __cplusplus
extern "C"{
#endif

PYJAVA_DLLSPEC JavaVM * pyjava_getJVM();
PYJAVA_DLLSPEC void pyjava_setJVM(JavaVM * jvm);
PYJAVA_DLLSPEC int pyjava_initJVM();
PYJAVA_DLLSPEC void pyjava_destroyJVM(void);
PYJAVA_DLLSPEC void pyjava_checkJNI(int enable);

PYJAVA_DLLSPEC void _pyjava_start_java(JNIEnv ** env, int * borrowed);
PYJAVA_DLLSPEC void _pyjava_end_java(JNIEnv ** env, int * borrowed);


#define PYJAVA_START_JAVA(ENVVAR) \
    JNIEnv *ENVVAR = NULL;\
    int _borrowed_##ENVVAR = -1; \
    _pyjava_start_java(& ENVVAR ,& _borrowed_##ENVVAR )

#define PYJAVA_END_JAVA(ENVVAR)\
    if (ENVVAR && PYJAVA_ENVCALL(ENVVAR,ExceptionCheck)) { \
    PYJAVA_ENVCALL(ENVVAR,ExceptionDescribe);\
    abort();\
    }\
    _pyjava_end_java(& ENVVAR, &_borrowed_##ENVVAR )

#ifdef __cplusplus
#define PYJAVA_ENVCALL(ENV,FUNC,...) (*ENV).FUNC(__VA_ARGS__)
#else
#define PYJAVA_ENVCALL(ENV,FUNC,...) (**ENV).FUNC(ENV, ##__VA_ARGS__)
#endif

#define PYJAVA_IGNORE_EXCEPTION(env) \
    if (PYJAVA_ENVCALL(env,ExceptionCheck)){ \
        PYJAVA_ENVCALL(env,ExceptionClear); \
    }

#define PYJAVA_MULTITHREADING
#ifndef PYJAVA_MULTITHREADING

#define PYJAVA_YIELD_GIL(STATE)
#define PYJAVA_RESTORE_GIL(STATE)

#else

#define PYJAVA_YIELD_GIL(STATE) \
    PyThreadState *STATE; \
    STATE = PyEval_SaveThread()
#define PYJAVA_RESTORE_GIL(STATE) \
    PyEval_RestoreThread(STATE)

#endif

#ifdef __cplusplus
}
#endif

#endif // JVM_H

