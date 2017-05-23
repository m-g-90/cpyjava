#ifndef PYJAVA_PYJAVA_H
#define PYJAVA_PYJAVA_H

#include <Python.h>
#include <jni.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct PyJavaObject {
    PyObject_HEAD
    jobject obj;
} PyJavaObject;


JavaVM * pyjava_getJVM();
void pyjava_setJVM(JavaVM * jvm);
int pyjava_initJVM();
void pyjava_enter();
void pyjava_exit();

void _pyjava_start_java(JNIEnv ** env, int * borrowed);
void _pyjava_end_java(JNIEnv ** env, int * borrowed);


#define PYJAVA_START_JAVA(ENVVAR) \
    JNIEnv *ENVVAR;\
    int _borrowed_##ENVVAR; \
    _pyjava_start_java(& ENVVAR ,& _borrowed_##ENVVAR )

#define PYJAVA_END_JAVA(ENVVAR)\
    if (PYJAVA_ENVCALL(ENVVAR,ExceptionCheck)) { \
    PYJAVA_ENVCALL(ENVVAR,ExceptionDescribe);\
    abort();\
    }\
    _pyjava_end_java(& ENVVAR, &_borrowed_##ENVVAR )

#define PYJAVA_YIELD_GIL(STATE) \
    PyThreadState *STATE; \
    STATE = PyEval_SaveThread()
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


#define PYJAVA_RESTORE_GIL(STATE) \
    PyEval_RestoreThread(STATE)


#define PYJAVA_JVMFROMENV(jvm,env,errreturn) \
	if (!env){\
		errreturn;\
	}\
	JavaVM * jvm;\
	env->GetJavaVM(&jvm);\
	if (!jvm){\
		errreturn;\
	}
#ifdef __cplusplus
#define PYJAVA_ENVCALL(ENV,FUNC,...) (*ENV).FUNC(__VA_ARGS__)
#else
#define PYJAVA_ENVCALL(ENV,FUNC,...) (**ENV).FUNC(ENV, ##__VA_ARGS__)
#endif

#define PYJAVA_IGNORE_EXCEPTION(env) \
    if (PYJAVA_ENVCALL(env,ExceptionCheck)){ \
        PYJAVA_ENVCALL(env,ExceptionClear); \
    }

PyMODINIT_FUNC PyInit_cpyjava(void);

#ifdef __cplusplus
}
#endif

#endif
