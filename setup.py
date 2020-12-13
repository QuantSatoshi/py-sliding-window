import os

from setuptools import Extension, setup

try:
    import numpy
except ImportError:
    numpy = None

define_macros = []
include_dirs = []
if numpy is not None:
    numpy_include = os.path.join(os.path.dirname(numpy.__file__), "core", "include")
    if os.path.exists(numpy_include):
        define_macros.append(("USE_NUMPY", 1))
        include_dirs.append(numpy_include)
setup(
    name="pyslidingwindow",
    version="0.0.1",
    description="Sliding window of floats wrapper to C++",
    author="Pat Miller",
    license="Copyright 2020, Quant Satoshi, All rights reserved",
    ext_modules=[
        Extension(
            "pyslidingwindow",
            sources=["src/slidingwindow.cc", "src/slidingWindowArr.cc"],
            depends=["src/slidingWindowArr.h"],
            define_macros=define_macros,
            include_dirs=include_dirs,
        ),
    ],
)
