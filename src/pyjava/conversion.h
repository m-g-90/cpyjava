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

#ifndef PYJAVA_CONVERSION_H
#define PYJAVA_CONVERSION_H

#include "pyjava/config.h"
#include "pyjava/pyjava.h"

#ifdef __cplusplus
extern "C"{
#endif



int pyjava_asJObject(JNIEnv * env, PyObject * obj, jclass klass, char ntype, jvalue * ret);
PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj);
PyObject * pyjava_asUndecoratedObject(JNIEnv *env, PyObject * obj);
PyObject * pyjava_asWrappedObject(JNIEnv * env, PyObject * obj);
PyObject * pyjava_asUnconvertedWrappedObject(JNIEnv * env,jobject obj);

int pyjava_exception_java2python(JNIEnv * env);

PyObject * pyjava_compile(const char * code,PyObject * locals,int eval);
void pyjava_conversion_forceInit(JNIEnv * env);

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
