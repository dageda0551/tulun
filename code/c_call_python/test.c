#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "python2.7/Python.h"
int main(int argc, char** argv)
{
    int arg0 = 0,arg1 = 0;
    if(argc == 3){
        arg0 = atoi(argv[1]);
        arg1 = atoi(argv[2]);
    }else{
        printf("please input 2 args!\n");
        return -1;
    }

    Py_Initialize();
    if ( !Py_IsInitialized() )
        return -1;

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

    PyObject *pModule;
    PyObject *pFunction;
    PyObject *pArgs;
    PyObject *pRetValue;
    pModule = PyImport_ImportModule("math_test");
    if(!pModule){
        printf("import python failed!\n");
        return -1;
    }

    pFunction = PyObject_GetAttrString(pModule, "add_func");
    if(!pFunction){
        printf("get python function failed!\n");
        return -1;
    }
    pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", arg0));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", arg1));

    pRetValue = PyObject_CallObject(pFunction, pArgs);
    printf("%d + %d = %ld\n", arg0, arg1, PyInt_AsLong(pRetValue));
    Py_DECREF(pModule);
    Py_DECREF(pFunction);
    Py_DECREF(pArgs);
    Py_DECREF(pRetValue);
    Py_Finalize();
    return 0;
}
