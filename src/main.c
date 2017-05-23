

#include "pyjava/pyjava.h"
#include <stdio.h>

int main(int argc,const char ** argv) {

    PyImport_AppendInittab("cpyjava", &PyInit_cpyjava);
    Py_Initialize();
    pyjava_initJVM();
    if (pyjava_getJVM()){
        printf("created a jvm\n");
    } else {
        printf("failed to create a jvm\n");
    }

    pyjava_enter();

    PyRun_InteractiveLoop(stdin,"???");

    pyjava_exit();

    Py_Finalize();
    return 0;
}

