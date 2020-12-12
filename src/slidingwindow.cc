#include <Python.h>
#ifdef USE_NUMPY
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"
#endif

#include "slidingWindowArr.h"
#include <vector>
#include <string>

// Handy debug macro
#define PP(x)  do { fprintf(stderr,#x": "); PyObject_Print((x),stderr,0); puts(""); } while(0)

typedef struct
{
  PyObject_HEAD
  int i;
  PyObject *window;
} WindowIterator;

static void
iter_dealloc(PyObject *pySelf)
{
  WindowIterator *self = reinterpret_cast<WindowIterator *>(pySelf);
  Py_XDECREF(self->window);
}

static PyObject *
iter_iter(PyObject *pySelf)
{
  // By convention, iterating on an iterator returns the iterator
  Py_INCREF(pySelf);
  return pySelf;
}

static PyObject *
iter_iternext(PyObject *pySelf)
{
  WindowIterator *iter = reinterpret_cast<WindowIterator *>(pySelf);

  // Call the item getter with i.  On error, just clear it and return nullptr (indicates we're done)
  PyObject *result = iter->window->ob_type->tp_as_sequence->sq_item(iter->window,iter->i);
  if (!result)
  {
    PyErr_Clear();
    return nullptr;
  }

  // Go to the next slot
  iter->i++;

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

  static bool getarg(int *p, PyObject *arg)
  {
    *p = PyLong_AsLong(arg);
    return !PyErr_Occurred();
  }
  static bool getarg(float *p, PyObject *arg)
  {
    *p = PyFloat_AsDouble(arg);
    return !PyErr_Occurred();
  }
  static bool getarg(double *p, PyObject *arg)
  {
    *p = PyFloat_AsDouble(arg);
    return !PyErr_Occurred();
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
    static char *kwlist[] = {(char *)"maxLen", (char*)"initializer", nullptr};

    // Set up the object.  Good to use nullptr here so if we bail out
    // and delete, then the object is in a reasonable state
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    self->object = nullptr;

    // Now we parse the arguments
    int maxLen;
    PyObject* initializer = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|O", kwlist,
                                   &maxLen,&initializer))
    // You can do some value checking, or trap an exception here if your
    // class handles its own errors
    if (maxLen < 0)
    {
      PyErr_SetString(PyExc_ValueError, "bad maxLen");
      return -1;
    }

    self->object = new SlidingWindowArr<T>(maxLen);

    // We may have an initializer
    if (initializer) {
      PyObject* iterator = /*owned*/ PyObject_GetIter(initializer);
      if (!iterator) {
	PyErr_Format(PyExc_RuntimeError,"initializer does not support iteration");
	return -1;
      }
      PyObject* item = nullptr;
      while ((item = PyIter_Next(iterator))) {
	PyObject* result = push(pySelf,item);
	Py_DECREF(item);
	Py_XDECREF(result);
	if (!result) {
	  Py_DECREF(iterator);
	  return -1;
	}
      }
      PyErr_Clear();
      Py_DECREF(iterator);
    }

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
    T x = self->object->first();
    return pythonize(&x);
  }

  static PyObject *last(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    T x = self->object->last();
    return pythonize(&x);
  }

  static PyObject *get(PyObject *pySelf, PyObject *args)
  {

    // Get an integer and return an error if arg is wrong type
    int i;
    if (!PyArg_ParseTuple(args, "i", &i))
      return nullptr;

    return item(pySelf,i);
  }

  static PyObject *push(PyObject *pySelf, PyObject *arg)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);

    // Get a value of the right type
    T x;
    if (!getarg(&x, arg))
      return nullptr;

    // Fetch the value
    try
    {
      self->object->push(x);
      Py_INCREF(pySelf);
      return pySelf;
    }
    catch (const std::exception &e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return nullptr;
    }
  }

#ifdef USE_NUMPY
  static int numpy_type_of(int*) {
    return NPY_INT;
  }
  static int numpy_type_of(float*) {
    return NPY_FLOAT;
  }
  static int numpy_type_of(double*) {
    return NPY_DOUBLE;
  }
#endif
  
  static PyObject *raw(PyObject *pySelf, PyObject *Py_UNUSED(ignored))
  {
    #ifdef USE_NUMPY
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    // Get raw pointer and figure out how many real values we have
    T* p = self->object->toUnorderedArr();
    npy_intp dims[1] = {self->object->getLength()};
    PyObject* a /*owned*/ = PyArray_SimpleNewFromData(1, dims, numpy_type_of(p), p);
    if (!a) return nullptr;

    // We want the array to keep the sliding window alive
    Py_INCREF(pySelf);
    PyArray_SetBaseObject(reinterpret_cast<PyArrayObject*>(a),pySelf);
    return a;
    #else
    PyErr_SetString(PyExc_NotImplementedError,"not built with numpy, so raw is not available");
    return nullptr;
    #endif
  }

  // We can build a Python iterator object to iterate over the elements in the window
  static PyObject *iter(PyObject *pySelf)
  {
    WindowIterator *result = PyObject_New(WindowIterator, &WindowIteratorType);
    if (result) {
      result->i = 0;
      Py_INCREF(result->window = pySelf);
    }
    return reinterpret_cast<PyObject *>(result);
  }

  static Py_ssize_t length(PyObject *pySelf)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return self->object->getLength();
  }

  static PyObject* repr(PyObject* pySelf)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
    return PyUnicode_FromFormat("<%s:%d of %d>",pySelf->ob_type->tp_name,self->object->getLength(),
				self->object->getMaxLen());
				
  }

  static PyObject* str(PyObject* pySelf)
  {
    PyObject* as_list /*owned*/ = PySequence_List(pySelf);
    if (!as_list) return nullptr;

    PyObject* as_str = PyObject_Str(as_list);
    Py_DECREF(as_list);
    return as_str;
  }

  static PyObject* item(PyObject *pySelf, Py_ssize_t i)
  {
    SlidingWindowObject *self = reinterpret_cast<SlidingWindowObject *>(pySelf);
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

public:
  wrapper(std::string const &type)
      : _type(type),
        _type_description(nullptr)
  {
    static PyMethodDef methods[] = {
        {"max_len", getMaxLen, METH_NOARGS, "maximum size of buffer"},
        {"length", getLength, METH_NOARGS, "working size of buffer"},
        {"get", get, METH_VARARGS, "Get the i'th value"},
        {"last", last, METH_NOARGS, "Get the last value"},
        {"first", first, METH_NOARGS, "Get the first value"},
        {"push", push, METH_O, "Insert new value at cursor"},
        {"raw", raw, METH_NOARGS, "return raw buffer as numpy array (if available)"},
        {nullptr} /* Sentinel */
    };

    static PySequenceMethods sequence;
    sequence.sq_length = length;
    sequence.sq_item = item;

    static std::string full_name = "pyslidingwindow.SlidingWindow" + _type;
    static PyTypeObject SlidingWindow = {
        PyVarObject_HEAD_INIT(NULL, 0)
    };
    SlidingWindow.tp_name = full_name.c_str();
    SlidingWindow.tp_basicsize = sizeof(SlidingWindowObject);
    SlidingWindow.tp_itemsize = 0;
    SlidingWindow.tp_dealloc = dealloc;
    SlidingWindow.tp_flags = Py_TPFLAGS_DEFAULT;
    SlidingWindow.tp_doc = "Very simple sliding window of values";
    SlidingWindow.tp_iter = iter;
    SlidingWindow.tp_methods = methods;
    SlidingWindow.tp_as_sequence = &sequence;
    SlidingWindow.tp_init = init;
    SlidingWindow.tp_new = PyType_GenericNew;
    SlidingWindow.tp_str = str;
    SlidingWindow.tp_repr = repr;

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
#ifdef USE_NUMPY
  // Initialize numpy API (if available)
  import_array();
  if (PyErr_Occurred()) return nullptr;
#endif
  
  // Window iterator shared by all
  if (PyType_Ready(&WindowIteratorType) < 0)
    return nullptr;

  // Set up the descriptions for the 3 types
  static wrapper<int> slidingwindow_int("Int");
  if (!slidingwindow_int.type())
    return 0;

  static wrapper<float> slidingwindow_float("Float");
  if (!slidingwindow_float.type())
    return 0;

  static wrapper<double> slidingwindow_double("Double");
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
    Py_DECREF(slidingwindow_int.type()); // We don't own this anymore!
    Py_DECREF(slidingwindow_float.type());
    Py_DECREF(m);
    return nullptr;
  }

  Py_INCREF(slidingwindow_double.type());
  if (PyModule_AddObject(m, "SlidingWindowDouble", (PyObject *)slidingwindow_double.type()) < 0)
  {
    Py_DECREF(slidingwindow_int.type()); // We don't own this anymore!
    Py_DECREF(slidingwindow_float.type());
    Py_DECREF(slidingwindow_double.type());
    Py_DECREF(m);
    return nullptr;
  }

  // We return the module object (with refcnt=1) and it will appear
  return m;
}
