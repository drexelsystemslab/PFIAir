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
        bool save_debug_models,
        bool profile,
        int max_iters) {

    Timer time_overall("Morphing source <--> target");
    if (profile)
        time_overall.start();

    // Ensure OpenVDB is set up before doing any morphing.
    // According to the docs, it is safe to call this more than once
    openvdb::initialize();

    // Preprocess both meshes
    LevelSet source_ls;
    LevelSet source_ls_hi_res;
    std::tie(source_ls, source_ls_hi_res) = preprocess_model(
        source_model, save_debug_models, profile);

    LevelSet target_ls;
    LevelSet target_ls_hi_res;
    std::tie(target_ls, target_ls_hi_res) = preprocess_model(
        target_model, save_debug_models, profile);

    // Output directories will be
    // output/morphs/{model1}-{model2}/
    std::string forwards_dir = "";
    std::string backwards_dir = "";
    if (save_debug_models) {
        mkdir_quiet(VDB_DIR);
        forwards_dir = 
            VDB_DIR + source_model.name + "-" + target_model.name + "/";
        mkdir_quiet(forwards_dir);

        backwards_dir = 
            VDB_DIR + target_model.name + "-" + source_model.name + "/";
        mkdir_quiet(backwards_dir);
    }

    // Perform the morphing
    Morph morpher;

    // Forwards Direction ====================
    Timer time_forward("Morphing source --> target");
    if (profile)
        time_forward.start();

    MorphStats forward_stats = morpher.morph(
        source_ls, target_ls_hi_res, forwards_dir, max_iters);
    forward_stats.set_names(source_model.name, target_model.name);

    if (profile)
        time_forward.stop();

    // Backwards direction ====================

    Timer time_backward("Morphing source <-- target");
    if (profile)
        time_backward.start();

    MorphStats backward_stats = morpher.morph(
        target_ls, source_ls_hi_res, backwards_dir, max_iters);
    backward_stats.set_names(target_model.name, source_model.name);

    if (profile) {
        time_backward.stop();
        time_overall.stop();
    }

    // Combine results into a pair of stats
    return MorphStatsPair(forward_stats, backward_stats);
}

// NON-EXPORTED HELPER FUNCTIONS ========================================

std::tuple<LevelSet, LevelSet> preprocess_model(
        const ModelInfo& model,
        bool save_obj,
        bool profile) {
    // Filenames:
    // <name>.obj - the processed obj file
    // <name>.vdb - the processed vdb file
    // hi_res_<name>.vdb - same as vdb but with much larger half bandwidth
    std::string output_obj = get_cache_name(model.obj_fname, "obj");
    std::string output_vdb = get_cache_name(model.obj_fname, "vdb");
    std::string output_vdb_hi_res = 
        PREPROCESS_CACHE + model.name + "_hi_res.vdb";

    std::cout << output_vdb_hi_res << std::endl;

    // Ensure we have a cache directory
    mkdir_quiet(PREPROCESS_CACHE);

    // Optional timer for processing
    std::string msg = "Preprocessing " + model.obj_fname;
    Timer timer(msg);
    if (profile)
        timer.start(); 

    // If we have pre-processed VDB files, just
    // load and return it.
    if (file_exists(output_vdb) && file_exists(output_vdb_hi_res)) {
        std::cout << "Using cached model: " << output_vdb << std::endl;

        // Load cached models
        LevelSet cached(output_vdb);
        LevelSet cached_hi_res(output_vdb_hi_res);

        if (profile)
            timer.stop();
        return std::make_tuple(cached, cached_hi_res);
    }

    // Otherwise, we need to preprocess the model
    Mesh mesh(model.obj_fname, model.is_open);
    std::cout << "Transforming Model..." <<  std::endl;
    mesh.preprocess_mesh();

    // Optionally save an OBJ file 
    if (save_obj) {
        std::cout << "Saving OBJ file: " << output_obj << std::endl;
        mesh.save_obj(output_obj);
    }

    // Convert Mesh -> LevelSet
    // the high res version is an attempt to limit the aliasing issues
    constexpr int HIGH_RES_LS_VOXELS = 100;
    LevelSet result = mesh.to_level_set();
    LevelSet result_hi_res = mesh.to_level_set(HIGH_RES_LS_VOXELS);

    // Save the vdb file
    std::cout << "Saving VDB file: " << output_vdb << std::endl;
    result.save(output_vdb);
    result_hi_res.save(output_vdb_hi_res);

    if (profile)
        timer.stop();

    return std::make_tuple(result, result_hi_res);
}

bool file_exists(std::string fname) {
    std::ifstream file(fname);
    if (file)
        return true;
    else
        return false;
}

void mkdir_quiet(std::string dirname) {
    if (!file_exists(dirname))
        mkdir(dirname.c_str(), S_IRWXU);
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
