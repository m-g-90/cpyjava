QT -= core
QT -= gui

CONFIG += c

TARGET = cpyjava
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/src/main.c

win32 {

    JDK_PATH = "C:/Program Files/Java/jdk1.8.0_111"
    PYTHON_PATH = "C:/Program Files/Python36"

    INCLUDEPATH += "$${JDK_PATH}/include"
    INCLUDEPATH += "$${JDK_PATH}/include/win32"
    INCLUDEPATH += "$${PYTHON_PATH}/include"

    LIBS += -L"$${PYTHON_PATH}/"
    LIBS += -L"$${PYTHON_PATH}/libs"
    LIBS += -L"$${JDK_PATH}/jre/bin/server"
    LIBS += -L"$${JDK_PATH}/lib"
    LIBS += -ljvm

}
unix {

    INCLUDEPATH += "/usr/lib/jvm/java-8-openjdk-amd64/include"
    INCLUDEPATH += "/usr/lib/jvm/java-8-openjdk-amd64/include/linux"

    LIBS += -L"/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server"
    LIBS += -L"/usr/lib/jvm/java-8-openjdk-amd64/lib"
    LIBS += -ljvm

}


include(.python.pri)
include(cpyjava.pri)

RESOURCES +=

DISTFILES +=

HEADERS +=
