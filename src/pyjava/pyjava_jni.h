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

#ifndef PYJAVA_JNI_H
#define PYJAVA_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C"{
#endif


/*
 * Class:     -
 * Method:    registerObject
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL pyjava_registerObject(JNIEnv *,jstring name,jobject object);




#ifdef __cplusplus
}
#endif


#endif // PYJAVA_JNI_H
