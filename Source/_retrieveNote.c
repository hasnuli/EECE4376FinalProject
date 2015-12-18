#include <Python.h>
#include <numpy/arrayobject.h>
#include "retrieveNote.c"

static char module_docstring[] =
"This module provides an interface for printing to LCD using C.";
static char* retrieveNote_docstring[] =
"Determine the correct pitch and appropiate frequency of a played note.";

static PyObject *retrieveNote_retrieveNote(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] =
{
    {"retrieveNote", retrieveNote_retrieveNote, METH_VARARGS, retrieveNote_docstring},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_retrieveNote(void)
{
    PyObject *m = Py_InitModule3("_retrieveNote", module_methods, module_docstring);
    
    if (m == NULL)
        return;

    // Load numpy functionality.
    import_array();
}

static PyObject *retrieveNote_retrieveNote(PyObject *self, PyObject *args)
{
    double pitch, highBound, lowBound, mid;
    PyObject *note;
    
    // Parse the input tuple
    if (!PyArg_ParseTuple(args, "ddddO", &pitch, &highBound, &lowBound, &mid,
                          &note))
        return NULL;
    
    // Interpret the input objects as numpy arrays.
    PyObject *note_array = PyArray_FROM_OTF(note, NPY_STRING, NPY_IN_ARRAY);
    
    // If that didn't work, throw an exception.
    if (note == NULL)
    {
        Py_XDECREF(note);
        return NULL;
    }
    
    // Call the external C function to compute the pitch.
    retrieveNote();
    
    // Clean up.
    Py_DECREF(note);
    
    // Build the output tuple
    PyObject *ret = Py_BuildValue("dddd0", pitch, highBound, lowBound, mid, note);
    
    return ret;
}