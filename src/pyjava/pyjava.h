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

#ifndef PYJAVA_PYJAVA_H
#define PYJAVA_PYJAVA_H

#include "config.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct PyJavaObject {
    PyObject_HEAD
    jobject obj;
} PyJavaObject;


void pyjava_enter();
void pyjava_exit();

/*
 * Class:     -
 * Method:    registerObject
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL pyjava_registerObject(JNIEnv *,jobject dont_care, jstring name,jobject object);


#define PYJAVA_YIELD_GIL(STATE) \
    PyThreadState *STATE; \
    STATE = PyEval_SaveThread()

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


PyMODINIT_FUNC PyInit_cpyjava(void);

#ifdef __cplusplus
}
#endif

#endif
