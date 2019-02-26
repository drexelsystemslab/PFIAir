# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the Cython interface (defined in pfimorph.pxd)
in a Python function.
"""
import json
import os
from lsmorph cimport morph_cpp
from lsmorph cimport ModelInfo
from lsmorph cimport MorphStatsPair

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
            # Maximum value difference for a single voxel from current frame
            # to another frame over time
            'max_diff_prev': list(stats.curve_max_diff_from_prev),
            'max_diff_target': list(stats.curve_max_diff_from_target),
            # Number of voxels different between frames over time
            'voxels_diff_prev': list(stats.curve_voxels_diff_from_prev),
            'voxels_diff_target': list(stats.curve_voxels_diff_from_target),
        }

    @property
    def to_dict(self):
        """
        Return a dict representation of the
        """
        stats_dict = {
            'stat_labels': self.COLUMNS,
            'source_name': self.source_name,
            'target_name': self.target_name,
            'forward_stats': self.forward_stats,
            'backward_stats': self.backward_stats,
            'forward_curves': self.forward_curves,
            'backward_curves': self.backward_curves,
        }
        return stats_dict

    def __reduce__(self):
        """
        After much struggling, I found that this was the correct method
        to implement for pickling a Cython object with struct fields
        """
        # first element is the constructor, StatPair()
        # second is constructor arguments (none in this case)
        # third is the state to pass into __setstate__()
        return (self.__class__, tuple(), self.to_dict)

    def __getstate__(self):
        """
        Needed for pickling
        """
        return self.to_dict

    def __setstate__(self, state):
        """
        Needed for pickling and loading JSON cache files
        """ 
        cdef MorphStats forwards
        forwards.source_name = state['source_name']
        forwards.target_name = state['target_name']
        self.set_stats_array(&forwards, state['forward_stats'])
        self.set_stats_curves(&forwards, state['forward_curves'])

        cdef MorphStats backwards
        backwards.source_name = state['target_name']
        backwards.target_name = state['source_name']
        self.set_stats_array(&backwards, state['backward_stats'])
        self.set_stats_curves(&backwards, state['backward_curves'])

        cdef MorphStatsPair pair
        pair.forwards = forwards
        pair.backwards = backwards
        self.pair = pair

    cdef set_stats_array(self, MorphStats* stats, stat_list):
        """
        Unpack a row of stats into one of the MorphStatsPairs

        This modifies the stats object, hence we need to pass in a pointer
        """
        [
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
        ] = stat_list

    cdef set_stats_curves(self, MorphStats* stats, curve_dict):
        """
        Unpact the curves dict

        This modifies the stats object, hence we need to pass in a pointer
        """
        stats.curve_max_curvature = curve_dict['max_curvature']
        stats.curve_frame_max_curvature = curve_dict['frame_max_curvature']
        stats.curve_delta_curvature = curve_dict['delta_curvature']
        stats.curve_delta_value = curve_dict['delta_value']
        stats.curve_cfl_iters = curve_dict['cfl_iters']
        stats.curve_surface_voxels = curve_dict['surface_voxels']
        stats.curve_max_diff_from_prev = curve_dict['max_diff_prev']
        stats.curve_max_diff_from_target = curve_dict['max_diff_target']
        stats.curve_voxels_diff_from_prev = curve_dict['voxels_diff_prev']
        stats.curve_voxels_diff_from_target = curve_dict['voxels_diff_target']

    @property
    def json_fname(self):
        """
        Make a filename for the json cache (basename only)
        """
        return "{}-{}.json".format(self.source_name, self.target_name)

    def save_json(self, json_dir):
        """
        Save a .json file of these stats into a specified directory
        """
        full_fname = os.path.join(json_dir, self.json_fname)
        with open(full_fname, 'w') as json_file:
            json.dump(self.to_dict, json_file)

    @classmethod
    def from_dict(cls, state_dict):
        stats = cls()
        stats.__setstate__(state_dict)
        return stats

cdef class Morpher:
    """
    OOP interface into the C++ morphing code
    """ 
    # Information that must be provided by the caller
    cdef ModelInfo source_model
    cdef ModelInfo target_model

    # The result of morphing
    cdef MorphStatsPair result

    def set_source_info(self, name, vdb_fname, high_res_fname):
        """
        Set information about the source model
        """
        self.source_model.name = name
        self.source_model.vdb_fname = vdb_fname
        self.source_model.high_res_fname = high_res_fname

    def set_target_info(self, name, vdb_fname, high_res_fname):
        """
        Set information about the target model
        """
        self.target_model.name = name
        self.target_model.vdb_fname = vdb_fname
        self.target_model.high_res_fname = high_res_fname

    def morph(
            self, 
            save_debug_models=False, 
            profile=False, 
            max_iters=500):
        """
        Morph the two models
        """
        result = morph_cpp(
            self.source_model,
            self.target_model,
            save_debug_models,
            profile,
            max_iters)

        stat_pair = StatPair()
        stat_pair.set_pair(result)
        return stat_pair
