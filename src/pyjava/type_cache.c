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


#include "pyjava/type_cache.h"
#include "pyjava/type.h"
#include "pyjava/method_cache.h"
#include "pyjava/config.h"
#include "pyjava/jvm.h"

#ifdef __cplusplus
extern "C"{
#endif



static struct PyJavaType * typemap[PYJAVA_CFG_TYPE_CACHE_BUCKETS];
struct PyJavaType * pyjava_typecache_find(JNIEnv * env, jclass klass){

    jint hash = pyjava_method_cache_identityHash(env,klass);

    PyJavaType * type = typemap[(size_t)hash%PYJAVA_CFG_TYPE_CACHE_BUCKETS];

    while (type){
        if (PYJAVA_ENVCALL(env,IsSameObject,type->klass,klass)){
            Py_IncRef((PyObject*)type);
            return type;
        }
        type = type->_tc_next;
    }

    return NULL;

}
void pyjava_typecache_register(JNIEnv * env, struct PyJavaType * type){

    if (type->_tc_next){
        return;
    }

    jint hash = pyjava_method_cache_identityHash(env,type->klass);

    PyJavaType * next = typemap[(size_t)hash%PYJAVA_CFG_TYPE_CACHE_BUCKETS];

    Py_IncRef((PyObject*)type);
    typemap[(unsigned)hash%PYJAVA_CFG_TYPE_CACHE_BUCKETS] = type;

    type->_tc_next = next;

}
void pyjava_typecache_gc(){

    /*
    for (size_t i = 0;i<PYJAVA_CFG_TYPE_CACHE_BUCKETS;i++){
        PyJavaType * prev = NULL;
        PyJavaType * cur = typemap[i];
        while (cur){
            if (cur->pto.ob_base.ob_base.ob_refcnt==1){
                PyJavaType * del = cur;
                cur = cur->_tc_next;
                if (prev) {
                    prev->_tc_next = cur;
                } else {
                    typemap[i] = cur;
                }
                Py_DecRef((PyObject*)del);
            } else {
                prev = cur;
                cur = cur->_tc_next;
            }
        }
    }
    */

}

void pyjava_typecache_reset(){

    /*
    for (size_t i = 0;i<PYJAVA_CFG_TYPE_CACHE_BUCKETS;i++){
        PyJavaType * cur = typemap[i];
        while (cur){
            PyJavaType * del = cur;
            cur = cur->_tc_next;
            Py_DecRef((PyObject*)del);
        }
        typemap[i] = NULL;
    }
    */

}

#ifdef __cplusplus
}
#endif






