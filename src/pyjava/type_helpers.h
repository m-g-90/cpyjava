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

#ifdef __cplusplus
extern "C"{
#endif

//TODO handle possible size differences of long
static PyObject * pyjava_callHelperInt(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
    (void)klass;
    PYJAVA_YIELD_GIL(gstate);
    long long tmp = PYJAVA_ENVCALL(env,CallIntMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLongLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperLong(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    long long tmp = PYJAVA_ENVCALL(env,CallLongMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLongLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperShort(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallShortMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperByte(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallByteMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperChar(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    wchar_t str[2];
    str[0] = PYJAVA_ENVCALL(env,CallCharMethodA,obj,meth,args);
    str[1] = 0;
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyUnicode_FromWideChar(str,1);
}
static PyObject * pyjava_callHelperBool(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallShortMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyBool_FromLong(tmp);
}
static PyObject * pyjava_callHelperFloat(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    double tmp = PYJAVA_ENVCALL(env,CallFloatMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyFloat_FromDouble(tmp);
}
static PyObject * pyjava_callHelperDouble(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    double tmp = PYJAVA_ENVCALL(env,CallDoubleMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyFloat_FromDouble(tmp);
}
static PyObject * pyjava_callHelperVoid(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    PYJAVA_ENVCALL(env,CallVoidMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    Py_RETURN_NONE;
}
static PyObject * pyjava_callHelperObject(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)klass;
    PYJAVA_YIELD_GIL(gstate);
    jobject ret = PYJAVA_ENVCALL(env,CallObjectMethodA,obj,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * pret = pyjava_asPyObject(env,ret);
    if (ret){
        PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
    }
    return pret;
}
static PyObject * pyjava_callHelperConstructor(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    jobject ret = PYJAVA_ENVCALL(env,NewObjectA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * pret = pyjava_asPyObject(env,ret);
    if (ret){
        PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
    }
    return pret;
}


static PyObject * pyjava_callHelperStaticInt(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    long long tmp = PYJAVA_ENVCALL(env,CallStaticIntMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLongLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperStaticLong(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    long long tmp = PYJAVA_ENVCALL(env,CallStaticLongMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLongLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperStaticShort(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallStaticShortMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperStaticByte(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallStaticByteMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * ret = PyLong_FromLong(tmp);
    return ret;
}
static PyObject * pyjava_callHelperStaticChar(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    wchar_t str[2];
    str[0] = PYJAVA_ENVCALL(env,CallStaticCharMethodA,klass,meth,args);
    str[1] = 0;
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyUnicode_FromWideChar(str,1);
}
static PyObject * pyjava_callHelperStaticBool(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    long tmp = PYJAVA_ENVCALL(env,CallStaticShortMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyBool_FromLong(tmp);
}
static PyObject * pyjava_callHelperStaticFloat(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    double tmp = PYJAVA_ENVCALL(env,CallStaticFloatMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyFloat_FromDouble(tmp);
}
static PyObject * pyjava_callHelperStaticDouble(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    double tmp = PYJAVA_ENVCALL(env,CallStaticDoubleMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    return PyFloat_FromDouble(tmp);
}
static PyObject * pyjava_callHelperStaticVoid(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    PYJAVA_ENVCALL(env,CallStaticVoidMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    Py_RETURN_NONE;
}
static PyObject * pyjava_callHelperStaticObject(JNIEnv * env,jmethodID meth,jobject obj,jclass klass,jvalue * args){
(void)obj;
    PYJAVA_YIELD_GIL(gstate);
    jobject ret = PYJAVA_ENVCALL(env,CallStaticObjectMethodA,klass,meth,args);
    PYJAVA_RESTORE_GIL(gstate);
    if (pyjava_exception_java2python(env)){
        return NULL;
    }
    PyObject * pret = pyjava_asPyObject(env,ret);
    if (ret){
        PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
    }
    return pret;
}


static PyObject *  pyjava_getFieldObject(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jobject ret = PYJAVA_ENVCALL(env,GetObjectField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = pyjava_asPyObject(env,ret);
    }
    if (ret){
        PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
    }
    return pret;
}
static void pyjava_setFieldObject(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetObjectField,obj,field,val->l);
}
static PyObject *  pyjava_getFieldBool(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jboolean ret = PYJAVA_ENVCALL(env,GetBooleanField,obj,field);
    if (!pyjava_exception_java2python(env)){
        if (ret){
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }
    return NULL;
}
static void pyjava_setFieldBool(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetBooleanField,obj,field,val->z);
}
static PyObject *  pyjava_getFieldByte(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jbyte ret = PYJAVA_ENVCALL(env,GetByteField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setFieldByte(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetByteField,obj,field,val->b);
}
static PyObject *  pyjava_getFieldChar(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jchar ret = PYJAVA_ENVCALL(env,GetCharField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        wchar_t s[2];
        s[0] = ret;
        s[1] = 0;
        pret = PyUnicode_FromWideChar(s,1);
    }
    return pret;
}
static void pyjava_setFieldChar(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetCharField,obj,field,val->c);
}
static PyObject *  pyjava_getFieldShort(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jshort ret = PYJAVA_ENVCALL(env,GetShortField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setFieldShort(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetShortField,obj,field,val->s);
}
static PyObject *  pyjava_getFieldInt(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jint ret = PYJAVA_ENVCALL(env,GetIntField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setFieldInt(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetIntField,obj,field,val->i);
}
static PyObject *  pyjava_getFieldLong(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jlong ret = PYJAVA_ENVCALL(env,GetLongField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setFieldLong(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetLongField,obj,field,val->j);
}
static PyObject *  pyjava_getFieldFloat(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jfloat ret = PYJAVA_ENVCALL(env,GetFloatField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyFloat_FromDouble(ret);
    }
    return pret;
}
static void pyjava_setFieldFloat(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetFloatField,obj,field,val->f);
}
static PyObject *  pyjava_getFieldDouble(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)klass;
    jdouble ret = PYJAVA_ENVCALL(env,GetDoubleField,obj,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyFloat_FromDouble(ret);
    }
    return pret;
}
static void pyjava_setFieldDouble(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)klass;
    PYJAVA_ENVCALL(env,SetDoubleField,obj,field,val->d);
}


static PyObject *  pyjava_getStaticFieldObject(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jobject ret = PYJAVA_ENVCALL(env,GetStaticObjectField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = pyjava_asPyObject(env,ret);
    }
    if (ret){
        PYJAVA_ENVCALL(env,DeleteLocalRef,ret);
    }
    return pret;
}
static void pyjava_setStaticFieldObject(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticObjectField,klass,field,val->l);
}
static PyObject *  pyjava_getStaticFieldDouble(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jdouble ret = PYJAVA_ENVCALL(env,GetStaticDoubleField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyFloat_FromDouble(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldDouble(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticDoubleField,klass,field,val->d);
}
static PyObject *  pyjava_getStaticFieldFloat(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jdouble ret = PYJAVA_ENVCALL(env,GetStaticFloatField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyFloat_FromDouble(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldFloat(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticFloatField,klass,field,val->f);
}

static PyObject *  pyjava_getStaticFieldLong(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jlong ret = PYJAVA_ENVCALL(env,GetStaticLongField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldLong(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticLongField,klass,field,val->j);
}
static PyObject *  pyjava_getStaticFieldInt(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jlong ret = PYJAVA_ENVCALL(env,GetStaticIntField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldInt(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticIntField,klass,field,val->i);
}
static PyObject *  pyjava_getStaticFieldShort(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jshort ret = PYJAVA_ENVCALL(env,GetStaticShortField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldShort(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticShortField,klass,field,val->s);
}
static PyObject *  pyjava_getStaticFieldByte(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jlong ret = PYJAVA_ENVCALL(env,GetStaticByteField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        pret = PyLong_FromLongLong(ret);
    }
    return pret;
}
static void pyjava_setStaticFieldByte(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticByteField,klass,field,val->b);
}
static PyObject *  pyjava_getStaticFieldBool(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jboolean ret = PYJAVA_ENVCALL(env,GetStaticBooleanField,klass,field);
    if (!pyjava_exception_java2python(env)){
        if (ret){
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }
    return NULL;
}
static void pyjava_setStaticFieldBool(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticBooleanField,klass,field,val->z);
}
static PyObject *  pyjava_getStaticFieldChar(JNIEnv* env,jfieldID field,jobject obj,jclass klass){
(void)obj;
    jchar ret = PYJAVA_ENVCALL(env,GetStaticCharField,klass,field);
    PyObject * pret = NULL;
    if (!pyjava_exception_java2python(env)){
        wchar_t s[2];
        s[0] = ret;
        s[1] = 0;
        pret = PyUnicode_FromWideChar(s,1);
    }
    return pret;
}
static void pyjava_setStaticFieldChar(JNIEnv* env,jfieldID field,jobject obj,jclass klass,jvalue * val){
(void)obj;
    PYJAVA_ENVCALL(env,SetStaticCharField,klass,field,val->c);
}

#ifdef __cplusplus
}
#endif
