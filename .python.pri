
win32 {

    PYTHON_PATH = "C:/Program Files/Python36"
    
    
    INCLUDEPATH += "$${PYTHON_PATH}/include"
    LIBS += -L"$${PYTHON_PATH}/"
    LIBS += -L"$${PYTHON_PATH}/libs"

}
unix {    

    isEmpty(PYTHON_CONFIG) {
        PYTHON_CONFIG = python3.5-config
    }

    message(PYTHON_CONFIG = $$PYTHON_CONFIG)

    QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
    QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
    
}



