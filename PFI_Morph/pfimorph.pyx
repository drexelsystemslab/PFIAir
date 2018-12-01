# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the Cython interface (defined in pfimorph.pxd)
in a Python function.
"""
from pfimorph cimport morph_cpp

def morph(src_obj, target_obj, cache=True):
    return morph_cpp(src_obj, target_obj, cache)
