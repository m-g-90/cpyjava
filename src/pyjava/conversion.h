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

#include "pyjava/pyjava.h"

#ifdef __cplusplus
extern "C"{
#endif

int pyjava_asJObject(JNIEnv * env, PyObject * obj, jclass klass, char ntype, jvalue * ret);
PyObject * pyjava_asPyObject(JNIEnv * env, jobject obj);

int pyjava_exception_java2python(JNIEnv * env);

#ifdef __cplusplus
}
#endif

#endif
