
#include "pyjava/type_extensions.h"
#include "pyjava/method_cache.h"
#include "pyjava/jvm.h"

#ifdef __cplusplus
extern "C"{
#endif


static Py_ssize_t java_util_Map_Length(PyJavaObject *o){
    PYJAVA_START_JAVA(env);
    jclass klass = pyjava_is_map_class(env);
    if (klass){
        jmethodID mid = PYJAVA_ENVCALL(env,GetMethodID,klass,"size","()I");
        if (mid){
            if (o && o->obj){
                Py_ssize_t ret = (Py_ssize_t) PYJAVA_ENVCALL(env,CallIntMethod,o->obj,mid);
                if (PYJAVA_ENVCALL(env,ExceptionCheck)) {
                    PYJAVA_ENVCALL(env,ExceptionClear);
                    ret = 0;
                }
                return ret;
            }
        }
    }
    PYJAVA_END_JAVA(env);
    return 0;
}
static PyObject * java_util_Map_Get(PyJavaObject *o,PyObject * key){
    PYJAVA_START_JAVA(env);
    jclass klass = pyjava_is_map_class(env);
    if (klass){
        jmethodID mid = PYJAVA_ENVCALL(env,GetMethodID,klass,"get","(Ljava/lang/Object;)Ljava/lang/Object;");
        if (mid){
            if (o && o->obj){
                jvalue jkey;
                if(pyjava_asJObject(env,key,pyjava_object_class(env),'L',&jkey)){
                    jobject ret = PYJAVA_ENVCALL(env,CallObjectMethodA,o->obj,mid,&jkey);
                    PYJAVA_ENVCALL(env,DeleteLocalRef,jkey.l);
                    if (pyjava_exception_java2python(env)) {
                        return NULL;
                    }
                    PyObject * pret = pyjava_asPyObject(env,ret);
                    PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
                    return pret;
                }
            }
        }
    }
    PYJAVA_END_JAVA(env);
    return 0;
}

static PyMappingMethods java_util_Map = {
    (lenfunc)&java_util_Map_Length,
    (binaryfunc)&java_util_Map_Get
};

void pyjava_init_type_extensions(JNIEnv * env,PyJavaType * type){
    if (PYJAVA_ENVCALL(env,IsSameObject,type->klass,pyjava_is_map_class(env))){
        type->pto.tp_as_mapping = &java_util_Map;
    }
}

#ifdef __cplusplus
}
#endif
