from distutils.core import setup, Extension
from Cython.Build import cythonize
import os
import multiprocessing

MORPH_DIR = 'MorphingUsingLevelSets'
BINVOX_DIR = 'VoxelConvert'

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
MORPH_FNAMES = [
    'morph_extension.cpp',
    'Timer.cpp',
    'Mesh.cpp',
    'LevelSet.cpp',
    'BoundingBox.cpp',
    'PCACalculator.cpp',
    'Morph.cpp',
    'MorphStats.cpp',
    'EnergyCalculator.cpp'
]

BINVOX_FNAMES = [
    'BinaryToLevelSet.cpp'
]

MORPH_SOURCES = [os.path.join(MORPH_DIR, x) for x in MORPH_FNAMES]
BINVOX_SOURCES = [os.path.join(BINVOX_DIR, x) for x in BINVOX_FNAMES]

# Declare an extension module
morph_ext = Extension(
    # This labels the resulting module, pfimorph.so
    'pfimorph',
    # List the main pyx file and all .cpp files needed
    sources=['pfimorph.pyx'] + MORPH_SOURCES,
    # We need to compile against OpenVDB and other C++ libraries
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    # This is a C++, not C library.
    language='c++')

binvox_ext = Extension(
    'binvox2vdb',
    sources=['binvox2vdb.pyx'] + BINVOX_SOURCES,
    libraries=['openvdb', 'tbb', 'Half', 'boost_iostreams'],
    language='c++')

# Setup like any other python library
setup(ext_modules=cythonize([binvox_ext, morph_ext]))
