#ifndef MORPH_EXTENSION_H
#define MORPH_EXTENSION_H
#include <string>
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
    bool cache=true,
    bool save_debug_models=false,
    bool profile=false);


// NON-EXPORTED HELPER FUNCTIONS ========================================


/**
 * Preprocess the model. If caching is enabled and an existing .vdb
 * file exists for this model, it will be loaded with no preprocessing.
 * If caching is disabled, the .vdb files will be ignored
 * 
 * There is a separate debug flag for saving an obj file representation
 * of the preprocessed model
 *
 * In both cases, models will be written to {PREPROCESS_CACHE}/
 *
 * If the profile flag is set, each step in the morphing process will be
 * timed and output will be written to stdout
 */
LevelSet preprocess_model(
    const ModelInfo& model,
    bool cache=true,
    bool save_obj=false,
    bool profile=false);

/**
 * Check if a file exists. This is useful for caching
 */
bool file_exists(std::string fname);

/**
 * Rename old/path/here/foo.obj -> {PREPROCESS_CACHE}/foo.{new_extension}
 */
std::string get_cache_name(
    std::string original_name, std::string new_extension);

#endif
