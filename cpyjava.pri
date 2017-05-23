


INCLUDEPATH += $$PWD/src/

SOURCES += $$PWD/src/pyjava/pyjava.c 
SOURCES += $$PWD/src/pyjava/type_cache.c 
SOURCES += $$PWD/src/pyjava/method_cache.c
SOURCES += $$PWD/src/pyjava/type.c
SOURCES += $$PWD/src/pyjava/conversion.c
SOURCES += $$PWD/src/pyjava/memory.c
HEADERS += $$PWD/src/pyjava/pyjava.h 
HEADERS += $$PWD/src/pyjava/type_cache.h 
HEADERS += $$PWD/src/pyjava/config.h 
HEADERS += $$PWD/src/pyjava/method_cache.h 
HEADERS += $$PWD/src/pyjava/pyjava_jni.h
HEADERS += $$PWD/src/pyjava/type_helpers.hc
HEADERS += $$PWD/src/pyjava/type.h
HEADERS += $$PWD/src/pyjava/conversion.h
HEADERS += $$PWD/src/pyjava/memory.h

INCLUDEPATH += "$${JDK_PATH}/include"
INCLUDEPATH += "$${PYTHON_PATH}/include"

unix {
    DEFINES += PYJAVA_JVM_DLOPEN
	INCLUDEPATH += "$${JDK_PATH}/include/linux"
}
win32 {

    DEFINES += PYJAVA_JVM_FORCELINK
    INCLUDEPATH += "$${JDK_PATH}/include/win32"

    LIBS += -L"$${PYTHON_PATH}/"
    LIBS += -L"$${PYTHON_PATH}/libs"
    LIBS += -L"$${JDK_PATH}/jre/bin/server"
    LIBS += -L"$${JDK_PATH}/lib"
    LIBS += -ljvm
}
