#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "morph_extension.h"
#include "Mesh.h"
#include "Timer.h"
#include "Morph.h"

// EXPORTED FUNCTIONS ===================================================

MorphStatsPair morph_cpp(
        const ModelInfo& source_model,
        const ModelInfo& target_model,
        bool cache,
        bool save_debug_models,
        bool profile) {

    // Ensure OpenVDB is set up before doing any morphing.
    // According to the docs, it is safe to call this more than once
    openvdb::initialize();

    // Preprocess both meshes
    LevelSet source_ls = preprocess_model(
        source_model, cache, save_debug_models, profile);
    LevelSet target_ls = preprocess_model(
        target_model, cache, save_debug_models, profile);

    // Output directories will be
    // output/morphs/{model1}-{model2}/
    std::string forwards_dir = "";
    std::string backwards_dir = "";
    if (save_debug_models) {
        forwards_dir = 
            VDB_DIR + source_ls.get_name() + "-" + target_ls.get_name() + "/";
        backwards_dir = 
            VDB_DIR + target_ls.get_name() + "-" + source_ls.get_name() + "/";
    }

    // Perform the morphing
    Morph morpher;
    MorphStats forward_stats = morpher.morph(
        source_ls, target_ls, forwards_dir);
    MorphStats backward_stats = morpher.morph(
        target_ls, source_ls, forwards_dir);

    return MorphStatsPair(forward_stats, backward_stats);
}

// NON-EXPORTED HELPER FUNCTIONS ========================================

LevelSet preprocess_model(
        const ModelInfo& model,
        bool cache, 
        bool save_obj,
        bool profile) {
    // The two possible output filenames
    std::string output_obj = get_cache_name(model.obj_fname, "obj");
    std::string output_vdb = get_cache_name(model.obj_fname, "vdb");

    // Ensure we have a cache directory
    mkdir(PREPROCESS_CACHE.c_str(), S_IRWXU);

    // Optional timer for processing
    std::string msg = "Preprocessing " + model.obj_fname;
    Timer timer(msg);
    if (profile)
        timer.start(); 

    // If caching is enabled and we have a pre-processed VDB, just
    // load and return it.
    if (cache && file_exists(output_vdb)) {
        std::cout << "Using cached model: " << output_vdb << std::endl;
        if (profile)
            timer.stop();
        return LevelSet(output_vdb);
    }

    // Otherwise, we need to preprocess the model
    Mesh mesh(model.obj_fname, model.is_open);
    mesh.set_name(model.name);
    std::cout << "Transforming Model..." <<  std::endl;
    mesh.preprocess_mesh();

    // Optionally save an OBJ file 
    if (save_obj) {
        std::cout << "Saving OBJ file: " << output_obj << std::endl;
        mesh.save_obj(output_obj);
    }

    // Convert Mesh -> LevelSet
    LevelSet result = mesh.to_level_set();

    // Save the 
    if (cache) {
        std::cout << "Saving VDB file: " << output_vdb << std::endl;
        result.save(output_vdb);
    }

    if (profile)
        timer.stop();

    return result;
}

bool file_exists(std::string fname) {
    std::ifstream file(fname);
    if (file)
        return true;
    else
        return false;
}

std::string get_cache_name(
        std::string original_name, std::string new_extension) {
    // Strip off the old path
    int last_slash = original_name.find_last_of("/");
    std::string basename = original_name.substr(last_slash + 1);
    
    // Change the extension
    int dot_pos = basename.find_last_of(".");
    std::string name = basename.substr(0, dot_pos);

    return PREPROCESS_CACHE + name + "." + new_extension;
}
