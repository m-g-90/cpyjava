#ifndef TYPE_EXTENSIONS_H
#define TYPE_EXTENSIONS_H

#include "pyjava/type.h"

#ifdef __cplusplus
extern "C"{
#endif

void pyjava_init_type_extensions(JNIEnv *env, PyJavaType * type);

#ifdef __cplusplus
}
#endif


#endif // TYPE_EXTENSIONS_H
