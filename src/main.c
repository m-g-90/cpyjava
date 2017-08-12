

#include "pyjava/pyjava.h"
#include "pyjava/selftest.h"
#include "pyjava/memory.h"
#include "pyjava/jvm.h"
#include <stdio.h>

int main(int argc,const char ** argv) {
    (void)argc;
    (void)argv;
    pyjava_checkJNI(1);

    pyjava_freestr(pyjava_dedupstaticstr("fdsasas"));

    PyImport_AppendInittab("cpyjava", &PyInit_cpyjava);
    Py_Initialize();
    PyEval_InitThreads();

    pyjava_enter();

    {
        PyObject * globals = PyDict_New();
        PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
        const char ** selftest = (const char **)pyjava_selftests;
        while (*selftest){

            PyRun_String(*selftest,Py_file_input,globals,globals);
            if (PyErr_Occurred()){
                PyErr_Print();
                printf("cpyjava selftest failed\n");
                if (argc > 1){
                    return -1;
                }
            }
            selftest++;
        }
    }

    if (argc == 1){
        PyRun_InteractiveLoop(stdin,"???");
    }

    pyjava_exit();

    Py_Finalize();
    return 0;
}

