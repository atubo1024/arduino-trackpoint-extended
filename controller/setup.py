#coding=utf-8

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import shutil

setup(
    name = 'core',
    ext_modules = cythonize(Extension(
        'core', 
        [ 'core.pyx', 'helper.c', '../serialframe.c' ], 
    ))
)

