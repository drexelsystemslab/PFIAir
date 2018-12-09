# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the public C++ interface from morph_extension.h 
in a Cython one

This module is imported by pfimorph.pyx
"""
from libcpp.string cimport string
cdef extern from "MorphingUsingLevelSets/morph_extension.h":

    # Input Info ============================================

    cdef struct ModelInfo:
        string obj_fname
        string name
        bint is_open

    # Output info ===========================================

    cdef struct MorphStats:
        string source_name
        string target_name
        int cfl_count
        int time_steps
        int source_surface_count
        int target_surface_count
        int abs_diff_count
        double src_tar_avg
        double total_curvature
        double weighted_total_curvature
        double max_curvature
        double total_value
        double weighted_total_value
        double total_energy
        double evolving_avg

    cdef struct MorphStatsPair:
        MorphStats forwards
        MorphStats backwards
        double average_energy() except +

    # Morph Function =========================================== 

    cdef MorphStatsPair morph_cpp(
        ModelInfo source_model,
        ModelInfo target_model,
        bint cache,
        bint save_debug_models,
        bint profile) except +
