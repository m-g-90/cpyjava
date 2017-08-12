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

#ifndef PYJAVA_TYPE_CACHE_H
#define PYJAVA_TYPE_CACHE_H

#include <pyjava/config.h>

#ifdef __cplusplus
extern "C"{
#endif

struct PyJavaType;


struct PyJavaType * pyjava_typecache_find(JNIEnv * env, jclass klass);
void pyjava_typecache_register(JNIEnv * env, struct PyJavaType * type);
void pyjava_typecache_gc();
void pyjava_typecache_reset();

#ifdef __cplusplus
}
#endif

#endif // TYPE_CACHE_H
