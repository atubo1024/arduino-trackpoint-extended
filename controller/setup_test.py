#coding=utf-8

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

setup(
    name = 'test',
    ext_modules = cythonize(Extension(
        'test', 
        [ 'test.pyx', 'helper.c', '../serialframe.c', '../lease_square.c' ], 
    ))
)

