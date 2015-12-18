#include <Python.h>
#include "returnState.c"

static char module_docstring[] =
"This module provides an interface for printing to LCD using C.";
static char* retrieveNote_docstring[] =
"Returns the state of the program.";

static PyObject *rreturnState_returnState(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] =
{
    {"returnState", returnState_returnState, METH_VARARGS, returnState_docstring},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_returnState(void)
{
    PyObject *m = Py_InitModule3("_returnState", module_methods, module_docstring);
    
    if (m == NULL)
        return;
}

static PyObject *returnState_returnState(PyObject *self, PyObject *args)
{
    int state;
    
    // Parse the input tuple
    if (!PyArg_ParseTuple(args, "i", state))
        return NULL;
    
    // Call the external C function to compute the pitch.
    state = returnState();
    
    // Build the output tuple
    PyObject *ret = Py_BuildValue("i", state);
    
    return ret;
}