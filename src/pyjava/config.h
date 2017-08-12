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

#ifndef PYJAVA_CONFIG_H
#define PYJAVA_CONFIG_H

#include <Python.h>
#include <jni/jni.h>



#define PYJAVA_CFG_TYPE_CACHE_BUCKETS (1024*1024)
#define PYJAVA_MALLOC_OVERHEAD 100
#define PYJAVA_STRING_DEDUP_BUCKETS (1024*1024)
#define PYJAVA_OBJECT_DEDUP_BUCKETS (1024*1024)

#ifdef _MSC_VER
    #ifdef PYJAVA_EXPORT
        #define PYJAVA_DLLSPEC __declspec(dllexport)
    #else
        #define PYJAVA_DLLSPEC __declspec(dllimport)
    #endif
#else
    #define PYJAVA_DLLSPEC
#endif

#define _PYJAVA_TOSTRING(X) #X
#define PYJAVA_TOSTRING(X) _PYJAVA_TOSTRING(X)

#if defined(QT_DEBUG) || defined(DEBUG)
#define PYJAVA_SOFTASSERT(X) if (!(X)) { printf("\nSoft assertion failed:\n\tExpression: %s\n\tLocation: %s:%d\n",PYJAVA_TOSTRING(X),__FILE__,__LINE__); }
#else
#define PYJAVA_SOFTASSERT(X)
#endif

#if defined(QT_DEBUG) || defined(DEBUG)
#define PYJAVA_ASSERT(X) if (!(X)) { printf("\nAssertion failed:\n\tExpression: %s\n\tLocation: %s:%d\n",PYJAVA_TOSTRING(X),__FILE__,__LINE__); abort(); }
#else
#define PYJAVA_ASSERT(X)
#endif


#endif // CONFIG_H
