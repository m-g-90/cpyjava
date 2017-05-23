QT -= core
QT -= gui

CONFIG += c

TARGET = cpyjava
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += $$PWD/src/main.c


JDK_PATH = "C:/Program Files/Java/jdk1.8.0_111"
PYTHON_PATH = "C:/Program Files/Python36"


include(cpyjava.pri)
