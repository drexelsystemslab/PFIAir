#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "morph_extension.h"
#include "Mesh.h"
#include "Timer.h"

// EXPORTED FUNCTIONS ===================================================

double morph_cpp(
        std::string source_obj,
        std::string target_obj, 
        bool source_open,
        bool target_open,
        bool cache_objs,
        bool profile) {

    // Preprocess both meshes
    preprocess_model(source_obj, source_open, cache_objs, profile);
    preprocess_model(target_obj, target_open, cache_objs, profile);

    // Morph source -> target
    // Morph target -> source

    //Return the energy
    return 0.5;
}

// NON-EXPORTED HELPER FUNCTIONS ========================================

Mesh preprocess_model(
        std::string model_fname, 
        bool open_mesh, 
        bool cache, 
        bool profile) {
    // Get the name of the model after preprocessing
    std::string cache_obj = get_cache_name(model_fname);

    // Ensure we have a cache directory
    mkdir(PREPROCESS_CACHE.c_str(), S_IRWXU);

    // Optional timer for processing
    std::string msg = "Preprocessing " + model_fname;
    Timer timer(msg);
    if (profile)
        timer.start(); 

    Mesh processed;
    if (cache && file_exists(cache_obj)) {
        // If a cache .obj exists, load that model
        std::cout << "Using cached model: " << cache_obj << std::endl;
        // preprocessed models are always closed meshes so open_mesh = false
        processed = Mesh(cache_obj, false);
    } else {
        // Otherwise, load and process the model
        processed = Mesh(model_fname, open_mesh);
        std::cout << "Transforming model..." << std::endl;
        processed.preprocess_mesh(); 
        
        // Save a .obj file if caching is enabled
        if (cache) {
            std::cout << "Saving cache file: " << cache_obj << std::endl;
            processed.save_obj(cache_obj);
        }
    }

    // Stop the timeer
    if (profile)
        timer.stop();

    return processed;
}

bool file_exists(std::string fname) {
    std::ifstream file(fname);
    if (file)
        return true;
    else
        return false;
}

std::string get_cache_name(std::string original_name) {
    int last_slash = original_name.find_last_of("/");
    std::string basename = original_name.substr(last_slash + 1);
    return PREPROCESS_CACHE + basename;
}
