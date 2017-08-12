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

#ifndef PYJAVA_PYJAVA_H
#define PYJAVA_PYJAVA_H

#include "config.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct PyJavaObject {
    PyObject_HEAD
    jobject obj;
    void * _dedupll;
    union {
        struct {
            uint32_t decoself : 1; // if 1 then this object will not be affected by decorators.
            uint32_t reserved : 31;
        } entries;
        uint32_t all;
    } flags;
} PyJavaObject;


PYJAVA_DLLSPEC void pyjava_enter();
PYJAVA_DLLSPEC void pyjava_exit();

/*
 * Class:     -
 * Method:    registerObject
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)V
 */
#ifdef __cplusplus
extern "C"
#endif
PYJAVA_DLLSPEC void pyjava_registerObject(JNIEnv *,jobject dont_care, jstring name,jobject object);

typedef PyObject * (*pyjava_converter_j2p_t)(JNIEnv * env,jclass klass,jobject object);
typedef jobject (*pyjava_converter_p2j_t)(JNIEnv * env,jclass klass,PyObject * obj);
/**
 * @brief registerConversion allows the register a custom converter
 * @param env
 * @param klass
 * @param cj2p
 * @param cp2j
 */
PYJAVA_DLLSPEC void pyjava_registerConversion(JNIEnv * env,jclass klass,pyjava_converter_j2p_t cj2p,pyjava_converter_p2j_t cp2j);

typedef int (*pyjava_native_converter_j2p_t)(JNIEnv * env,char ntype,PyObject * object,jvalue * value);
PYJAVA_DLLSPEC void pyjava_registerNativeConversion(char ntype,pyjava_native_converter_j2p_t fnc);


PYJAVA_DLLSPEC PyObject * pyjava_getModule(void);

// blow must be compatible with PyMODINIT_FUNC
#ifdef __cplusplus
extern "C"
#endif
PYJAVA_DLLSPEC PyObject * PyInit_cpyjava(void);



#ifdef __cplusplus
}
#endif

#endif
