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

#ifndef PYJAVA_CONVERSION_H
#define PYJAVA_CONVERSION_H

#include "pyjava/config.h"

#ifdef __cplusplus
extern "C"{
#endif



int pyjava_asJObject(JNIEnv * env, PyObject * obj, jclass klass, char ntype, jvalue * ret);
PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj);
PyObject * pyjava_asWrappedObject(JNIEnv * env, PyObject * obj);

int pyjava_exception_java2python(JNIEnv * env);


typedef PyObject * (*pyjava_converter_j2p_t)(JNIEnv * env,jclass klass,jobject object);
typedef jobject (*pyjava_converter_p2j_t)(JNIEnv * env,jclass klass,PyObject * obj);
/**
 * @brief registerConversion allows the register a custom converter
 * @param env
 * @param klass
 * @param cj2p
 * @param cp2j
 */
void pyjava_registerConversion(JNIEnv * env,jclass klass,pyjava_converter_j2p_t cj2p,pyjava_converter_p2j_t cp2j);

typedef int (*pyjava_native_converter_j2p_t)(JNIEnv * env,char ntype,PyObject * object,jvalue * value);
void pyjava_registerNativeConversion(char ntype,pyjava_native_converter_j2p_t fnc);

typedef struct PyJavaConverter {
    pyjava_converter_j2p_t convj2p0;
    pyjava_converter_p2j_t convp2j0;
    pyjava_converter_j2p_t * convj2p1u;
    pyjava_converter_p2j_t * convp2j1u;
    int convcountj2p;
    int convcountp2j;
} PyJavaConverter;

#ifdef __cplusplus
}
#endif

#endif
