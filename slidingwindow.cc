#include <Python.h>

#include <vector>

// Some C++ class here
class SlidingWindow {
  unsigned bufsize_;
  unsigned winsize_;
  std::vector<double> data_;
  unsigned data_index_;
  unsigned insert_index_;
public:
  SlidingWindow(unsigned bufsize, unsigned winsize)
    : bufsize_(bufsize),
      winsize_(winsize),
      data_(bufsize),
      data_index_(0u),
      insert_index_(0u)
  {
  }

  double get(int i) const {
    if (i < 0u || (unsigned)i > winsize_) {
      throw std::runtime_error("index out of range");
    }
    return data_[(data_index_+i)%bufsize_];
  }

  void slide() {
    data_index_ = (data_index_+1)%bufsize_;
  }

  void insert(double x) {
    data_[insert_index_] = x;
    insert_index_ = (insert_index_+1)%bufsize_;
  }
};

// We need to describe the type and its functions to Python
// This is a wrapper object that will alias PyObject (that's what
// is in the head, basically points at a type description and
// has a reference count
typedef struct {
  PyObject_HEAD
  // Easiest way to control C++ objects is just to have a pointer
  // here.  Also possible to use placement new.  Just a matter of
  // efficiency (and putting the proper delete code in dealloc
  // below)
  SlidingWindow* object;
} SlidingWindowObject;

static void
SlidingWindow_dealloc(PyObject *pySelf)
{
  SlidingWindowObject* self = reinterpret_cast<SlidingWindowObject*>(pySelf);

  // Clean up any C++ stuff in the wrapper object
  delete (self->object);

  // And Python cleans up the rest
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int
SlidingWindow_init(PyObject *pySelf, PyObject *args, PyObject *kwds)
{
  // Argument keyword names (has to be char* because of very old C API
  static char* kwlist[] = {(char*)"bufsize",(char*)"winsize", nullptr};
  
  // Set up the object.  Good to use nullptr here so if we bail out
  // and delete, then the object is in a reasonable state
  SlidingWindowObject* self = reinterpret_cast<SlidingWindowObject*>(pySelf);
  self->object = nullptr;

  // Now we parse the arguments
  int bufsize,winsize;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist,
				   &bufsize, &winsize))
    return -1; // Signal an error (object will clean up just fine)

  // You can do some value checking, or trap an exception here if your
  // class handles its own errors
  if (bufsize <= 0 || winsize <= 0 || bufsize < winsize) {
    PyErr_SetString(PyExc_ValueError,"bad buf or win size");
    return -1;
  }

  self->object = new SlidingWindow(bufsize,winsize);
  return 0; // This says the init was completed without error
}

// We add in methods here.  We can either just clone the C++ API
// (doing that here) or map things onto special Python methods

static PyObject* SlidingWindow_slide(PyObject *pySelf, PyObject *Py_UNUSED(ignored)) {
  SlidingWindowObject* self = reinterpret_cast<SlidingWindowObject*>(pySelf);

  // Do the work on the C++ object inside the wrapper
  self->object->slide();

  // Every Python function has to return something, this will return None
  // which is a singleton (this macro gets the reference counting right)
  Py_RETURN_NONE;
}

static PyObject* SlidingWindow_insert(PyObject *pySelf, PyObject *args) {
  SlidingWindowObject* self = reinterpret_cast<SlidingWindowObject*>(pySelf);

  // Get a double and return an error if arg is wrong type
  double x;
  if (!PyArg_ParseTuple(args,"d",&x)) return nullptr;

  // Do the update on the object
  self->object->insert(x);
  
  Py_RETURN_NONE;
}

static PyObject* SlidingWindow_get(PyObject *pySelf, PyObject *args) {
  SlidingWindowObject* self = reinterpret_cast<SlidingWindowObject*>(pySelf);

  // Get an integer and return an error if arg is wrong type
  int i;
  if (!PyArg_ParseTuple(args,"i",&i)) return nullptr;

  // Fetch the value
  try {
    double x = self->object->get(i);

    // and now make it into a python Float (which is a C++ double)
    return PyFloat_FromDouble(x);
  }
  catch(const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError,e.what());
    return nullptr;
  }
}

static PyMethodDef SlidingWindow_methods[] = {
  // Very simple for methods with no arguments
  {"slide", SlidingWindow_slide, METH_NOARGS, "Move the window over 1 to the right"},
  {"insert", SlidingWindow_insert, METH_VARARGS, "Put a new value in buffer (wraps)"},
  {"get", SlidingWindow_get, METH_VARARGS, "get a value from the window"},
  {nullptr}  /* Sentinel */
};

static PyTypeObject SlidingWindow = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "slidingwindow.SlidingWindow",
    .tp_doc = "Very simple sliding window of values",
    .tp_basicsize = sizeof(SlidingWindowObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = SlidingWindow_init,
    .tp_dealloc = SlidingWindow_dealloc,
    .tp_methods = SlidingWindow_methods,
};



// this is stuff for the module....
const char* module_doc =
  "this is a longer doc string\n"
  "\n"
  "and more stuff here\n"
  ;

static PyMethodDef methods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef module_description = {
    PyModuleDef_HEAD_INIT,
    "slidingwindow",   /* name of module */
    module_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables
		 which this does (inside SlidingWindow type description).
	      */
    methods
};

PyMODINIT_FUNC  PyInit_slidingwindow(void) {
  // This sets up internal structures for Python so the type object is good to go
  if (PyType_Ready(&SlidingWindow) < 0) return nullptr;
  
  PyObject* m = PyModule_Create(&module_description);

  // Stuff in Python is reference counted... the "module" owns one copy, so
  // we set the refcnt to 1 here
  Py_INCREF(&SlidingWindow);
  if (PyModule_AddObject(m, "SlidingWindow", (PyObject *) &SlidingWindow) < 0) {
    Py_DECREF(&SlidingWindow); // We don't own this anymore!
    Py_DECREF(m);              // And the module isn't being returned
    return nullptr;
  }

  // We return the module object (with refcnt=1) and it will appear
  return m;
}
