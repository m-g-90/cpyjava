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

#ifndef PYJAVA_METHOD_CACHE_H
#define PYJAVA_METHOD_CACHE_H

#include <pyjava/config.h>

#ifdef __cplusplus
extern "C"{
#endif

jint pyjava_method_cache_identityHash(JNIEnv * env,jobject obj);
void pyjava_method_cache_reset(JNIEnv * env);
int pyjava_is_class(JNIEnv * env,jobject obj);
int pyjava_is_arrayclass(JNIEnv * env,jclass obj);
jclass pyjava_get_array_sub_Type(JNIEnv * env,jclass klass);
jclass pyjava_class_class(JNIEnv * env);
jclass pyjava_map_class(JNIEnv * env);
jclass pyjava_list_class(JNIEnv * env);
jclass pyjava_set_class(JNIEnv * env);
jclass pyjava_object_class(JNIEnv * env);
jclass pyjava_iterable_class(JNIEnv * env);
jclass pyjava_iterator_class(JNIEnv * env);
jclass pyjava_compareable_class(JNIEnv * env);
jclass pyjava_throwable_class(JNIEnv * env);
int pyjava_object_equal(JNIEnv * env,jobject o1,jobject o2);
jmethodID pyjava_compareable_compareTo_method(JNIEnv * env);

#ifdef __cplusplus
}
#endif

#endif // METHOD_CACHE_H
