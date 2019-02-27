#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "morph_extension.h"
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
    LevelSet source_ls(source_model.vdb_fname);
    LevelSet source_ls_hi_res(source_model.high_res_fname);

    LevelSet target_ls(target_model.vdb_fname);
    LevelSet target_ls_hi_res(target_model.high_res_fname);

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

    // Forwards Direction ------------------------------------------
    Timer time_forward("Morphing source --> target");
    if (profile)
        time_forward.start();

    MorphStats forward_stats = morpher.morph(
        source_ls, target_ls_hi_res, forwards_dir, max_iters);
    forward_stats.set_names(source_model.name, target_model.name);

    if (profile)
        time_forward.stop();

    // Backwards direction ----------------------------------------

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
