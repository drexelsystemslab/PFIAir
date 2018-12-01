from distutils.core import setup, Extension
from Cython.Build import cythonize

#TODO: Automatically search for C++ files
cpp_sources = [
    'MorphingUsingLevelSets/morph_extension.cpp'
]

# Declare a Cython extension module
morph_ext = cythonize(Extension(
    # This labels the resulting module, pfimorph.so
    'pfimorph',
    # List the main pyx file and all .cpp files needed
    sources=['pfimorph.pyx'] + cpp_sources,
    # We need to compile against OpenVDB and other C++ libraries
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    # This is a C++, not C library.
    language='c++'))

# Setup like any other python library
setup(ext_modules=morph_ext)
