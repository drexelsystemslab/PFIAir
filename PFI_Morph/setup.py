from distutils.core import setup, Extension
from Cython.Build import cythonize
import os
import multiprocessing

SRC_DIR = 'MorphingUsingLevelSets'

def find_sources():
    """
    Find source files automatically
    """
    for root, dirs, files in os.walk(SRC_DIR):
        for fname in files:
            if fname.endswith('.cpp'):
                yield os.path.join(root, fname)

# The source directory has old and new source code. For now, I'm manually
# filtering for source files
source_fnames = [
    'morph_extension.cpp',
    'Timer.cpp',
    'Mesh.cpp',
    'LevelSet.cpp',
    'BoundingBox.cpp',
    'PCACalculator.cpp'
]

cpp_sources = [os.path.join(SRC_DIR, x) for x in source_fnames]

# Declare an extension module
morph_ext = Extension(
    # This labels the resulting module, pfimorph.so
    'pfimorph',
    # List the main pyx file and all .cpp files needed
    sources=['pfimorph.pyx'] + cpp_sources,
    # We need to compile against OpenVDB and other C++ libraries
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    # This is a C++, not C library.
    language='c++')

# Cythonize it in parallel
cython_morph = cythonize(morph_ext, nthreads=multiprocessing.cpu_count())

# Setup like any other python library
setup(ext_modules=cython_morph)
