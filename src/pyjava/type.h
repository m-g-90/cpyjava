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

#ifndef PYJAVA_TYPE_H
#define PYJAVA_TYPE_H

#include "pyjava/pyjava.h"
#include "pyjava/conversion.h"

#ifdef __cplusplus
extern "C"{
#endif

struct PyJavaType;
typedef struct PyJavaParameter {
    const char * name;
    jclass klass;
    char ntype;
} PyJavaParameter;
typedef PyObject * (*pyjava_callHelper_t)(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args);
typedef union PyJavaModifiers {
        struct {
            jint unknown0 : 3;
            jint isStatic : 1;
            jint unknown5 : 28;
        };
        jint all;
} PyJavaModifiers;

typedef struct PyJavaMethod {
    const char * name;
    jmethodID methodid;
    PyJavaModifiers modifiers;
    jclass returnType;
    char returnNType;
    pyjava_callHelper_t callHelper;
    PyJavaParameter * parameters;
    int parametercount;
    struct PyJavaMethod * next;
} PyJavaMethod;

typedef PyObject * (* pyjava_getFieldHelper_t)(JNIEnv* env,jfieldID field,jobject obj,jclass klass);
typedef void (* pyjava_setFieldHelper_t)(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val);
typedef struct PyJavaField {
    const char * name;
    char ntype;
    jclass type;
    PyJavaModifiers modifiers;
    jfieldID fieldid;
    pyjava_getFieldHelper_t getter;
    pyjava_setFieldHelper_t setter;
    struct PyJavaField * next;
} PyJavaField;
typedef struct PyJavaDecoration {
    PyObject * tp_call;
    PyObject * tp_getattro;
    PyObject * tp_setattro;
    struct {
        uint32_t tp_call : 1;
        uint32_t tp_getattro : 1;
        uint32_t tp_setattro : 1;
        uint32_t reserved : 29;
    } inherited;
} PyJavaDecoration;

#define PYJAVA_SYMBOL_BUCKET_COUNT 32
typedef struct PyJavaType {
    PyTypeObject pto;
    jclass klass;
    jmethodID hashCode;
    jmethodID toString;
    jmethodID class_getName;
    struct PyJavaMethod * methods[PYJAVA_SYMBOL_BUCKET_COUNT];
    struct PyJavaField * fields[PYJAVA_SYMBOL_BUCKET_COUNT];
    PyObject * dir;
    struct PyJavaType * _tc_next;
    struct PyJavaConverter converter;
    struct PyJavaDecoration decoration;
    ternaryfunc tp_call_impl; //don't overwrite tp_call in type_extensions. overwrite this to ensure that decoration works.
    PyObject * (*tp_richcompare_impl)(PyObject * , PyObject *, int );
    jclass arrayklass;
    char arrayntype;
} PyJavaType;

int pyjava_isJavaClass(PyTypeObject * type);

PyObject * pyjava_callFunction(JNIEnv * env, PyObject * _obj,const char * name,PyObject * tuple);
int pyjava_hasFunction(JNIEnv * env, PyObject * _obj,const char * name);
PyObject * pyjava_getField(JNIEnv * env, PyObject * _obj,const char * name);
void pyjava_setField(JNIEnv * env, PyObject * _obj,const char * name,PyObject * val);
char pyjava_getNType(JNIEnv * env,jclass klass);
PyTypeObject * pyjava_classAsType(JNIEnv * env,jclass klass);
PyTypeObject * pyjava_classNameAsType(JNIEnv * env,PyObject * classloader,const char * classname);
void pyjava_inherit_decorators(PyTypeObject * type);

#ifdef __cplusplus
}
#endif

#endif
