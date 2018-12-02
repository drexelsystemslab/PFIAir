# cython: language_level = 2
# distutils: language = c++
"""
This module wraps the Cython interface (defined in pfimorph.pxd)
in a Python function.
"""
from pfimorph cimport morph_cpp
from pfimorph cimport ModelInfo
from pfimorph cimport MorphStatsPair

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
        return result.average_energy()
