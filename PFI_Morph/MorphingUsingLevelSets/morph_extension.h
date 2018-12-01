#ifndef MORPH_EXTENSION_H
#define MORPH_EXTENSION_H
#include <string>
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
 * This runs preprocess_model
 *
 * TODO: This should return a MorphStatsPair for reporting in Python land
 */
double morph_cpp(
    std::string source_obj,
    std::string target_obj, 
    bool cache_objs=true);


// NON-EXPORTED HELPER FUNCTIONS ========================================


/**
 * Preprocess a model, optionally saving the model to a cache
 *
 * TODO:
 */
void preprocess_model(std::string model, bool cache=true);

#endif
