# cython: language_level = 2
# distutils: language = c++
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair

cdef extern from "cpp/binvox2vdb/BinaryToLevelSet.h":
    ctypedef pair[int, int] RunLength
    ctypedef vector[RunLength] BinvoxData

    cdef cppclass BinaryToLevelSet:
        BinaryToLevelSet()
        void set_dimensions(int d, int w, int h)
        void set_translation(float x, float y, float z)
        void set_scale(float scale)
        void populate_grid(const BinvoxData& data)
        void convert(float half_bandwidth, int smoothing_steps)
        void save(string filename)
