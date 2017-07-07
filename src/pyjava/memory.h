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

#ifndef PYJAVA_MEMORY_H
#define PYJAVA_MEMORY_H
#include "Python.h"
#include <malloc.h>


#ifdef __cplusplus
extern "C"{
#endif
void * pyjava_malloc(size_t size);
void pyjava_free(void * ptr);

PyObject * pyjava_memory_statistics(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
