#ifndef MORPH_EXTENSION_H
#define MORPH_EXTENSION_H
#include <string>
#include <tuple>
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
    // Filename of VDB file on disk
    std::string vdb_fname;
    // Solid version of the VDB
    std::string high_res_fname;
    // Short name without path or extension
    std::string name;
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
