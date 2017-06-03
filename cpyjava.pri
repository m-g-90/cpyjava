


INCLUDEPATH += $$PWD/src/

SOURCES += $$PWD/src/pyjava/pyjava.c \ 
    $$PWD/src/pyjava/jvm.c \
    $$PWD/src/pyjava/type_extensions.c
SOURCES += $$PWD/src/pyjava/type_cache.c 
SOURCES += $$PWD/src/pyjava/method_cache.c
SOURCES += $$PWD/src/pyjava/type.c
SOURCES += $$PWD/src/pyjava/conversion.c
SOURCES += $$PWD/src/pyjava/memory.c
HEADERS += $$PWD/src/pyjava/pyjava.h \ 
    $$PWD/src/pyjava/jvm.h \
    $$PWD/src/pyjava/type_helpers.h \
    $$PWD/src/pyjava/type_extensions.h
HEADERS += $$PWD/src/pyjava/type_cache.h 
HEADERS += $$PWD/src/pyjava/config.h 
HEADERS += $$PWD/src/pyjava/method_cache.h 
HEADERS +=
HEADERS +=
HEADERS += $$PWD/src/pyjava/type.h
HEADERS += $$PWD/src/pyjava/conversion.h
HEADERS += $$PWD/src/pyjava/memory.h


unix {
    DEFINES += PYJAVA_JVM_FORCELINK PYJAVA_JVM_DLOPEN
}
win32 {
    DEFINES += PYJAVA_JVM_FORCELINK
}
