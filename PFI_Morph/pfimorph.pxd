# cython: language_level = 2
"""
This module wraps the public C++ interface from morph_extension.h 
in a Cython one

This module is imported by pfimorph.pyx
"""
from libcpp.string cimport string
cdef extern from "MorphingUsingLevelSets/morph_extension.h":
    cdef double morph_cpp(
        string source_obj, 
        string target_obj, 
        bint cache_objs)
