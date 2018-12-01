#ifndef MORPH_EXTENSION_H
#define MORPH_EXTENSION_H
#include <string>
#include "Mesh.h"

/**
 * This file defines the public methods for the morph extension module
 */

// preprocessed models will go here
static const std::string PREPROCESS_CACHE = "preprocessed/";

// EXPORTED FUNCTIONS ===================================================

/**
 * Morph source -> target and target -> source, returning the
 * energy.
 *
 * TODO: This should return a MorphStatsPair for reporting in Python land
 */
double morph_cpp(
        std::string source_obj,
        std::string target_obj, 
        bool source_open,
        bool target_open,
        bool cache_objs=true,
        bool profile=false);


// NON-EXPORTED HELPER FUNCTIONS ========================================


/**
 * Preprocess a model, optionally saving the model to a cache
 */
Mesh preprocess_model(
    std::string model_fname, 
    bool open_mesh, 
    bool cache=true,
    bool profile=false);

/**
 * Check if a file exists. This is useful for caching
 */
bool file_exists(std::string fname);

/**
 * Rename old/path/here/foo.obj -> {PREPROCESS_CACHE}/foo.obj
 */
std::string get_cache_name(std::string original_name);

#endif
