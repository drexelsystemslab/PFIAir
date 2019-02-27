from distutils.core import setup, Extension
from Cython.Build import cythonize
import os
import multiprocessing

MORPH_DIR = 'cpp/lsmorph'
BINVOX_DIR = 'cpp/binvox2vdb'

def find_sources(src_dir):
    """
    Find source files automatically
    """
    for root, dirs, files in os.walk(src_dir):
        for fname in files:
            if fname.endswith('.cpp'):
                yield os.path.join(root, fname)

MORPH_SOURCES = list(find_sources(MORPH_DIR))
BINVOX_SOURCES = list(find_sources(BINVOX_DIR))

# Declare an extension module
morph_ext = Extension(
    # This labels the resulting module
    'pfimorph.lsmorph',
    # List the main pyx file and all .cpp files needed
    sources=['pfimorph/lsmorph.pyx'] + MORPH_SOURCES,
    # We need to compile against OpenVDB and other C++ libraries
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    # This is a C++, not C library.
    language='c++')

binvox_ext = Extension(
    'pfimorph.binvox2vdb',
    sources=['pfimorph/binvox2vdb.pyx'] + BINVOX_SOURCES,
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    language='c++')

cython_modules = cythonize([binvox_ext, morph_ext], build_dir='build/cython')

# Setup like any other python library
setup(ext_modules=cython_modules, include_dirs=['.'])
