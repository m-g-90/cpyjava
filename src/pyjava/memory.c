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


#include "pyjava/memory.h"
#include "pyjava/config.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct mem_block {
    size_t size;
    struct mem_block * next;
}mem_block;

#define PYJAVA_MEMORY_DECL(I) \
    static mem_block * block##I = NULL; \
    static size_t block##I##_usecount = 0; \
    static size_t block##I##_freecount = 0;


PYJAVA_MEMORY_DECL(32)
PYJAVA_MEMORY_DECL(64)
PYJAVA_MEMORY_DECL(512)
PYJAVA_MEMORY_DECL(1024)
PYJAVA_MEMORY_DECL(4096)

int blockcount = 0;
static void * _pyjava_malloc(size_t size){
    mem_block * ret = (mem_block*)malloc(size+sizeof(mem_block));
    ret->size = size;
    ret->next = NULL;
    //printf("malloc %i\n",blockcount);
    blockcount++;
    return (void*) (ret+1);
}
static void _pyjava_free(void * ptr){
    mem_block * ret = (mem_block*)ptr;
    ret = ret-1;
    free(ret);
    blockcount--;
}

#define pyjava_malloc_size(X) \
    if (size<=X){\
        if (block##X){\
            mem_block * ret = block##X;\
            block##X = ret->next;\
            ret->next = NULL;\
            block##X##_usecount++;\
            block##X##_freecount--;\
            return (void*)(ret+1);\
        }\
        void * ret = _pyjava_malloc(X);\
        if (ret){\
            block##X##_usecount++; \
        }\
        return ret;\
    } else

void * pyjava_malloc(size_t size){
    pyjava_malloc_size(64)
    pyjava_malloc_size(32)
    pyjava_malloc_size(512)
    pyjava_malloc_size(1024)
    pyjava_malloc_size(4096)
    {
        return _pyjava_malloc(size);
    }
}

#define pyjava_malloc_free(X) \
    if ((ret->size == X) && (block##X##_freecount<(PYJAVA_MALLOC_OVERHEAD))){ \
        ret->next = block##X; \
        block##X = ret; \
        block##X##_usecount--;\
        block##X##_freecount++;\
    } else \

void pyjava_free(void * ptr){
    if (!ptr)
        return;
    mem_block * ret = ((mem_block*)ptr)-1;
    pyjava_malloc_free(32)
    pyjava_malloc_free(64)
    pyjava_malloc_free(512)
    pyjava_malloc_free(1024)
    pyjava_malloc_free(4096)
    {
        _pyjava_free(ptr);
    }
}

#define PYJAVA_MEMORY_APPEND(I) \
    PyUnicode_AppendAndDel(&ret,PyUnicode_FromFormat("\t%d: Used: %d(%d bytes) Free: %d(%d bytes) Overhead: %d bytes\n",I,block##I##_usecount,block##I##_usecount * I,block##I##_freecount,block##I##_freecount * I,(block##I##_usecount+block##I##_freecount)*sizeof(mem_block))); \
    tused += block##I##_usecount * I ;\
    tfree += block##I##_freecount * I ;\
    tover += (block##I##_usecount + block##I##_freecount) * sizeof(mem_block);

PyObject * pyjava_memory_statistics(const char * cmd){

    PyObject * ret = PyUnicode_FromString("Memory Statistics:\n");

    size_t tused = 0;
    size_t tfree = 0;
    size_t tover = 0;
    PYJAVA_MEMORY_APPEND(32)
    PYJAVA_MEMORY_APPEND(64)
    PYJAVA_MEMORY_APPEND(512)
    PYJAVA_MEMORY_APPEND(1024)
    PYJAVA_MEMORY_APPEND(4096)

    PyUnicode_AppendAndDel(&ret,PyUnicode_FromFormat("\tTotal: Used: %d bytes Free: %d bytes Overhead: %d bytes\n",tused,tfree,tover));

    if (cmd){
        if (!strcmp(cmd,"used")){
            Py_DecRef(ret);
            return PyLong_FromSize_t(tused);
        } else if (!strcmp(cmd,"all")){

        } else {
            Py_DecRef(ret);
            PyErr_SetString(PyExc_RuntimeWarning,"unknown command");
            return NULL;
        }
    }

    return ret;

}



#ifdef __cplusplus
}
#endif
