#include <Python.h>

#include "slidingWindowArr.h"
#include <vector>
#include <string>

typedef struct
{
  PyObject_HEAD
      PyObject *i;
  PyObject *getter;
} WindowIterator;

static void
iter_dealloc(PyObject *pySelf)
{
  WindowIterator *self = reinterpret_cast<WindowIterator *>(pySelf);
  Py_XDECREF(self->i);
  Py_XDECREF(self->getter);
}

static PyObject *
iter_iter(PyObject *pySelf)
{
  Py_INCREF(pySelf);
  return pySelf;
}

static PyObject *
iter_iternext(PyObject *pySelf)
{
  static PyObject *one = PyLong_FromLong(1);

  WindowIterator *self = reinterpret_cast<WindowIterator *>(pySelf);

  // Call the getter with i.  On error, just clear it and return nullptr (indicates we're done)
  PyObject *result = PyObject_CallFunctionObjArgs(self->getter, self->i, 0);
  if (!result)
  {
    PyErr_Clear();
    return nullptr;
  }

  // Go to the next slot
  PyObject *next = PyNumber_Add(self->i, one);
  Py_DECREF(self->i);
  self->i = next;

  return result;
}

PyTypeObject WindowIteratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "pyslidingwindow.SlidingWindowIterator",
    .tp_basicsize = sizeof(WindowIterator),
    .tp_itemsize = 0,
    .tp_dealloc = iter_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = iter_iter,
    .tp_iternext = iter_iternext,
    .tp_new = PyType_GenericNew,
};

template <class T>
class wrapper
{
  std::string _type;
  PyTypeObject *_type_description;

  // We need to describe the type and its functions to Python
  // This is a wrapper object that will alias PyObject (that's what
  // is in the head, basically points at a type description and
  // has a reference count
  typedef struct
  {
    PyObject_HEAD
        // Easiest way to control C++ objects is just to have a pointer
        // here.  Also possible to use placement new.  Just a matter of
        // efficiency (and putting the proper delete code in dealloc
        // below)
        SlidingWindowArr<T> *object;
  } SlidingWindowObject;

  static void
  dealloc(PyObject *pySelf)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);

    // Clean up any C++ stuff in the wrapper object
    delete (self->object);

    // And Python cleans up the rest
    Py_TYPE(self)->tp_free((PyObject *)self);
  }

  static int
  init(PyObject *pySelf, PyObject *args, PyObject *kwds)
  {
    // Argument keyword names (has to be char* because of very old C API
    static char *kwlist[] = {(char *)"maxLen", nullptr};

    // Set up the object.  Good to use nullptr here so if we bail out
    // and delete, then the object is in a reasonable state
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    self->object = nullptr;

    // Now we parse the arguments
    int maxLen;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist,
                                     &maxLen))
      return -1; // Signal an error (object will clean up just fine)

    // You can do some value checking, or trap an exception here if your
    // class handles its own errors
    if (maxLen < 0)
    {
      PyErr_SetString(PyExc_ValueError, "bad maxLen");
      return -1;
    }

    self->object = new SlidingWindowArr<T>(maxLen);
    return 0; // This says the init was completed without error
  }

  static PyObject *getMaxLen(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return PyLong_FromLong(self->object->getMaxLen());
  }

  static PyObject *getLength(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return PyLong_FromLong(self->object->getLength());
  }

  static PyObject *first(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return PyLong_FromLong(self->object->first());
  }

  static PyObject *last(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return PyLong_FromLong(self->object->last());
  }

  static PyObject *get(PyObject *pySelf, PyObject *args)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);

    // Get an integer and return an error if arg is wrong type
    int i;
    if (!PyArg_ParseTuple(args, "i", &i))
      return nullptr;

    // Fetch the value
    try
    {
      T x = self->object->get(i);

      // and now make it into a python object of the right type
      return pythonize(&x);
    }
    catch (const std::exception &e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return nullptr;
    }
  }

  static PyObject *push(PyObject *pySelf, PyObject *args)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);

    // Get a value of the right type
    T x;
    if (!getarg(&x, args))
      return nullptr;

    // Fetch the value
    try
    {
      self->object->push(x);
      Py_RETURN_NONE;
    }
    catch (const std::exception &e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return nullptr;
    }
  }

  static bool getarg(int *p, PyObject *args)
  {
    if (!PyArg_ParseTuple(args, "i", p))
      return false;
    return true;
  }
  static bool getarg(float *p, PyObject *args)
  {
    double x;
    if (!PyArg_ParseTuple(args, "d", &x))
      return false;
    *p = x;
    return true;
  }
  static bool getarg(double *p, PyObject *args)
  {
    if (!PyArg_ParseTuple(args, "d", p))
      return false;
    return true;
  }

  static PyObject *pythonize(int *p)
  {
    return PyLong_FromLong(*p);
  }
  static PyObject *pythonize(float *p)
  {
    return PyFloat_FromDouble(*p);
  }
  static PyObject *pythonize(double *p)
  {
    return PyFloat_FromDouble(*p);
  }

  // We can build a Python iterator object to iterate over the elements in the window
  static PyObject *iter(PyObject *pySelf)
  {
    WindowIterator *result = PyObject_New(WindowIterator, &WindowIteratorType);
    result->i = PyLong_FromLong(0);
    result->getter = PyObject_GetAttrString(pySelf, "get");
    if (!result->getter)
      return nullptr;

    return reinterpret_cast<PyObject *>(result);
  }

public:
  wrapper(std::string const &type)
      : _type(type),
        _type_description(nullptr)
  {
    static PyMethodDef methods[] = {
        {"max_len", getMaxLen, METH_NOARGS, "maximum size of buffer"},
        {"length", getLength, METH_NOARGS, "working size of buffer"},
        {"get", get, METH_VARARGS, "Get the i'th value"},
        {"last", last, METH_VARARGS, "Get the last value"},
        {"first", first, METH_VARARGS, "Get the first value"},
        {"push", push, METH_VARARGS, "Insert new value at cursor"},
        {nullptr} /* Sentinel */
    };

    static PyTypeObject SlidingWindow = {
        PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "pyslidingwindow.SlidingWindow",
        .tp_basicsize = sizeof(SlidingWindowObject),
        .tp_itemsize = 0,
        .tp_dealloc = dealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = "Very simple sliding window of values",
        .tp_iter = iter,
        .tp_methods = methods,
        .tp_init = init,
        .tp_new = PyType_GenericNew,
    };

    // Ready the type
    if (PyType_Ready(&SlidingWindow) >= 0)
    {
      _type_description = &SlidingWindow;
    }
  }

  PyTypeObject *type()
  {
    return _type_description;
  }
};

// this is stuff for the module....
const char *module_doc =
    "this is a longer doc string\n"
    "\n"
    "and more stuff here\n";

static PyMethodDef methods[] = {
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef module_description = {
    PyModuleDef_HEAD_INIT,
    "pyslidingwindow", /* name of module */
    module_doc,        /* module documentation, may be NULL */
    -1,                /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables
		 which this does (inside the wrapper<T> objects)
	      */
    methods};

PyMODINIT_FUNC PyInit_pyslidingwindow(void)
{
  // Window iterator shared by all
  if (PyType_Ready(&WindowIteratorType) < 0)
    return nullptr;

  // Set up the descriptions for the 3 types
  static wrapper<int> slidingwindow_int("int");
  if (!slidingwindow_int.type())
    return 0;

  static wrapper<float> slidingwindow_float("float");
  if (!slidingwindow_float.type())
    return 0;

  static wrapper<double> slidingwindow_double("double");
  if (!slidingwindow_double.type())
    return 0;

  // Generate a Python module to hold everything
  PyObject *m = PyModule_Create(&module_description);

  // Stuff in Python is reference counted... the "module" owns one copy, so
  // we set the refcnt to 1 here
  Py_INCREF(slidingwindow_int.type());
  if (PyModule_AddObject(m, "SlidingWindowInt", (PyObject *)slidingwindow_int.type()) < 0)
  {
    Py_DECREF(slidingwindow_int.type()); // We don't own this anymore!
    Py_DECREF(m);                        // And the module isn't being returned
    return nullptr;
  }

  Py_INCREF(slidingwindow_float.type());
  if (PyModule_AddObject(m, "SlidingWindowFloat", (PyObject *)slidingwindow_float.type()) < 0)
  {
    Py_DECREF(slidingwindow_float.type());
    Py_DECREF(m);
    return nullptr;
  }

  Py_INCREF(slidingwindow_double.type());
  if (PyModule_AddObject(m, "SlidingWindowDouble", (PyObject *)slidingwindow_double.type()) < 0)
  {
    Py_DECREF(slidingwindow_double.type());
    Py_DECREF(m);
    return nullptr;
  }

  // We return the module object (with refcnt=1) and it will appear
  return m;
}
