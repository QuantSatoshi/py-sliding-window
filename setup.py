from setuptools import setup, Extension

setup(
    name = 'pyslidingwindow',
    version = '0.0.1',
    description = 'Sliding window of floats wrapper to C++',
    author = 'Pat Miller',
    license = 'Copyright 2020, Quant Satoshi, All rights reserved',
    ext_modules = [Extension('pyslidingwindow',
                             sources=['src/slidingwindow.cc',
                                      'src/slidingWindowArr.cc'],
                             depends=['src/slidingWindowArr.h'],
    ),
    ]
)
    
