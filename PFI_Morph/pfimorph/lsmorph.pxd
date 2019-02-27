# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the public C++ interface from morph_extension.h 
in a Cython one

This module is imported by pfimorph.pyx
"""
from libcpp.string cimport string
from libcpp.vector cimport vector
cdef extern from "cpp/lsmorph/morph_extension.h":

    # Input Info ============================================

    cdef struct ModelInfo:
        string vdb_fname
        string high_res_fname
        string name

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
        vector[double] curve_max_curvature
        vector[double] curve_frame_max_curvature
        vector[double] curve_delta_curvature
        vector[double] curve_delta_value
        vector[double] curve_cfl_iters
        vector[double] curve_surface_voxels
        vector[double] curve_max_diff_from_prev
        vector[double] curve_max_diff_from_target
        vector[double] curve_voxels_diff_from_prev;
        vector[double] curve_voxels_diff_from_target;

    cdef struct MorphStatsPair:
        MorphStats forwards
        MorphStats backwards
        MorphStatsPair swapped()
        double average_energy() except +

    # Morph Function =========================================== 

    cdef MorphStatsPair morph_cpp(
        ModelInfo source_model,
        ModelInfo target_model,
        bint save_debug_models,
        bint profile,
        int max_iters) except +
