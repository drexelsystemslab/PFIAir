# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the Cython interface (defined in pfimorph.pxd)
in a Python function.
"""
from pfimorph cimport morph_cpp
from pfimorph cimport ModelInfo
from pfimorph cimport MorphStatsPair


cdef class StatPair:
    COLUMNS = [
        "Image",
        "CFL Iterations",
        "Time steps",
        "Source Voxel Count",
        "Target Voxel Count",
        "Abs diff",
        "Evolving Average",
        "Source-target Average",
        "Total Curvature",
        "Max Curvature",
        "Curvature / Area",
        "Total Value",
        "(Value / Area) * 100",
        "Total Energy",
        "Mean",
    ]

    cdef MorphStatsPair pair

    cdef set_pair(self, MorphStatsPair pair):
        self.pair = pair

    @property
    def source_name(self):
        return self.pair.forwards.source_name

    @property
    def target_name(self):
        return self.pair.forwards.target_name

    @property
    def average_energy(self):
        return self.pair.average_energy()

    @property
    def forward_stats(self):
        return self.stats_array(self.pair.forwards)

    @property
    def backward_stats(self):
        return self.stats_array(self.pair.backwards)

    def stats_array(self, MorphStats stats):
        return [
            stats.cfl_count,
            stats.time_steps,
            stats.source_surface_count,
            stats.target_surface_count,
            stats.abs_diff_count,
            stats.evolving_avg,
            stats.src_tar_avg,
            stats.max_curvature,
            stats.total_curvature,
            stats.weighted_total_curvature,
            stats.total_value,
            stats.weighted_total_value,
            stats.total_energy
        ]

cdef class Morpher:
    cdef ModelInfo source_model
    cdef ModelInfo target_model
    cdef MorphStatsPair result

    def set_source_info(self, obj_fname, name, is_open):
        """
        Set information about the source model
        """
        self.source_model.obj_fname = obj_fname
        self.source_model.name = name
        self.source_model.is_open = is_open

    def set_target_info(self, obj_fname, name, is_open):
        """
        Set information about the target model
        """
        self.target_model.obj_fname = obj_fname
        self.target_model.name = name
        self.target_model.is_open = is_open

    def morph(self, cache=True, save_debug_models=False, profile=False):
        """
        Morph the two models
        """
        result = morph_cpp(
            self.source_model,
            self.target_model,
            cache,
            save_debug_models,
            profile)

        stat_pair = StatPair()
        stat_pair.set_pair(result)
        return stat_pair
