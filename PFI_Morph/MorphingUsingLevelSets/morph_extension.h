#ifndef MORPH_EXTENSION_H
#define MORPH_EXTENSION_H
#include <string>
#include <tuple>
#include "Mesh.h"
#include "MorphStats.h"
#include "LevelSet.h"

/**
 * This file defines the public methods for the morph extension module
 */

// preprocessed models will go here
static const std::string PREPROCESS_CACHE = "output/preprocessed/";
static const std::string VDB_DIR = "output/morphs/";

// EXPORTED FUNCTIONS ===================================================

/**
 * Struct of information to pass to the morphing code
 */
struct ModelInfo {
    // Filename of OBJ file on disk, e.g. /path/to/foo.obj
    std::string obj_fname;
    // Short name e.g. foo
    std::string name;
    // True if this is an open mesh
    bool is_open;
};

/**
 * Morph source -> target and target -> source, returning the
 * energy.
 *
 * TODO: This should return a MorphStatsPair for reporting in Python land
 */
MorphStatsPair morph_cpp(
    const ModelInfo& source_model,
    const ModelInfo& target_model,
    bool save_debug_models=false,
    bool profile=false,
    int max_iters=500);


// NON-EXPORTED HELPER FUNCTIONS ========================================


/**
 * Preprocess the model. If existing .vdb
 * files exists for this model, it will be loaded with no preprocessing.
 * 
 * There is a separate debug flag for saving an obj file representation
 * of the preprocessed model. This is for debugging only
 *
 * In both cases, models will be written to {PREPROCESS_CACHE}/
 *
 * If the profile flag is set, each step in the morphing process will be
 * timed and output will be written to stdout
 *
 * This function returns to versions of the model: One with a half bandwidth
 * of 3 for using as the source model, and another one with a half bandwidth
 * much higher for use as the target model. The intent is to minimize aliasing
 * issues
 */
std::tuple<LevelSet, LevelSet> preprocess_model(
    const ModelInfo& model,
    bool save_obj=false,
    bool profile=false);

/**
 * Check if a file exists. This is useful for caching
 */
bool file_exists(std::string fname);

/**
 * Make a directory if it doesn't already exist
 */
void mkdir_quiet(std::string dirname);

/**
 * Rename old/path/here/foo.obj -> {PREPROCESS_CACHE}/foo.{new_extension}
 */
std::string get_cache_name(
    std::string original_name, std::string new_extension);

#endif
