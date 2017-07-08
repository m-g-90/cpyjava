

#include "pyjava/pyjava.h"

#include <stdio.h>

int main(int argc,const char ** argv) {
    (void)argc;
    (void)argv;

    PyImport_AppendInittab("cpyjava", &PyInit_cpyjava);
    Py_Initialize();
    PyEval_InitThreads();

    pyjava_enter();

    PyRun_InteractiveLoop(stdin,"???");

    pyjava_exit();

    Py_Finalize();
    return 0;
}

