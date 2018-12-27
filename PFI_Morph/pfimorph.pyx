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
    """
    Thin Python wrapper around a MorphStatsPair from C++ land.
    """
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
    def swapped(self):
        """
        Return the same information but with source and target models
        swapped.
        """
        new_pair = StatPair()
        new_pair.set_pair(self.pair.swapped())
        return new_pair

    @property
    def source_name(self):
        """
        Get the short name of the source model
        """
        return self.pair.forwards.source_name

    @property
    def target_name(self):
        """
        Get the short name of the target model
        """
        return self.pair.forwards.target_name

    @property
    def average_energy(self):
        """
        Average the energy between the two models
        """
        return self.pair.average_energy()

    @property
    def forward_stats(self):
        """
        get a list of the stats for putting into a HTML template
        """
        return self.stats_array(self.pair.forwards)

    @property
    def forward_curves(self):
        """
        Get the curves of curvature and other stats over time as the
        """
        return self.curve_dict(self.pair.forwards)

    @property
    def backward_stats(self):
        """
        get a list of the stats for putting into a HTML template
        """
        return self.stats_array(self.pair.backwards)

    @property
    def backward_curves(self):
        """
        Get the curves of curvature and other stats over time from
        the 
        """
        return self.curve_dict(self.pair.backwards)


    def stats_array(self, MorphStats stats):
        """
        Pull out stats and put them into a single Python list
        """
        return [
            stats.cfl_count,
            stats.time_steps,
            stats.source_surface_count,
            stats.target_surface_count,
            stats.abs_diff_count,
            stats.evolving_avg,
            stats.src_tar_avg,
            stats.total_curvature,
            stats.max_curvature,
            stats.weighted_total_curvature,
            stats.total_value,
            stats.weighted_total_value,
            stats.total_energy
        ]
    
    def curve_dict(self, MorphStats stats):
        """
        Convert the morphing curves
        These might help us find a better distance function than curvature
        alone
        """
        # For the below curves, time is measured in frames of the animation.
        return {
            # plot of maximum absolute mean curvature *per frame* over time
            'frame_max_curvature': list(stats.curve_frame_max_curvature),
            # cumulative max_curvature over time
            'max_curvature': list(stats.curve_max_curvature),
            # plot of total absolute delta curvature *per frame* over time
            'delta_curvature': list(stats.curve_delta_curvature),
            # plot of total absolute delta curvature *per frame* over time
            'delta_value': list(stats.curve_delta_value),
            # plot of CFL iterations *per frame* over time to identify
            # peaks
            'cfl_iters': list(stats.curve_cfl_iters),
            # plot of total surface voxels over time
            'surface_voxels': list(stats.curve_surface_voxels),
        }

cdef class Morpher:
    """
    OOP interface into the C++ morphing code
    """ 
    # Information that must be provided by the caller
    cdef ModelInfo source_model
    cdef ModelInfo target_model

    # The result of morphing
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
