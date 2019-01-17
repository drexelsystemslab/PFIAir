#include <iostream>

#include <sys/time.h>
#include <sys/resource.h>

#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>

#include "Mesh.h"
#include "Timer.h"
#include "Morph.h"
#include "morph_extension.h"


//const std::string MORPH_OUTPUT_DIR = "/Volumes/ExtHDD/Jeshur/new/morphs/";
const std::string OUTPUT_DIR = "output/";
const std::string INPUT_DIR = "original_objs/";
const std::string TEMP_OBJ1 = OUTPUT_DIR + "srt1.obj";
const std::string TEMP_OBJ2 = OUTPUT_DIR + "srt2.obj";

/**
 * Some functions in the existing code do not work well with open meshes.
 * They consume all memory until the process hits the OOM killer.
 *
 * This locks up my laptop for several minutes at a time, so I am
 * limiting memory usage as a precaution.
 */
void limit_memory(long megabytes) {
    rlimit mem_limit;
    long bytes = megabytes * 1024 * 1024;
    mem_limit.rlim_cur = bytes;
    mem_limit.rlim_max = bytes;
    // Limit the address space
    setrlimit(RLIMIT_AS, &mem_limit);
}

std::string model_names[] = {
    "cylinder", 
    "hand",  
    "stand", 
    "duck", 
    "mushroom-1", 
    "nail-1", 
    "swan", 
    "pebble",  
    "claw", 
    "container", 
    "winding-wheel", 
    "bird", 
    "man-gun", 
    "tool", 
    "modular-hand", 
    "beast", 
    "hammer", 
    "bishop", 
    "cup", 
    "flask", 
    "head",  
    "queen", 
    "table", 
    "spaceship"
};

void print_help() {
    std::cout << "PFI_Morph: similartiy using level set morphing" << std::endl
        << "Usage: PFI_Morph [cmd]" << std::endl
        << std::endl
        << "Commands:" << std::endl
        << "open_mesh\tRun experiment for morphing open meshes" << std::endl
        << std::endl
        << "help\t\tPrint this help message and quit" << std::endl;
}


/**
 * Test of scan-converting a non-watertight model
 */
int load_open_mesh(int argc, const char* argv[]) {
    // Limit memory usage to 1 GB as a safety precaution. I don't want to 
    // lock up my laptop again.
    limit_memory(1000);

    const std::string INPUT_PATH = "open_mesh_objs/"; 
    const std::string OUTPUT_PATH = "output/open_mesh/";
    // Original OBJ files
    const std::string SOURCE_OBJ = INPUT_PATH + "source.obj";
    const std::string TARGET_OBJ = INPUT_PATH + "target.obj";
    
    // Use the entry point that Cython uses 
    ModelInfo source_info{
        .obj_fname = SOURCE_OBJ,
        .name = "source",
        .is_open = true,
    };
    ModelInfo target_info{
        .obj_fname = TARGET_OBJ,
        .name = "target",
        .is_open = true,
    };

    // Run the morphing in full debug mode
    // Note: This runs a full-length morph, up to 500 frames
    try {
        MorphStatsPair results = morph_cpp(
            source_info,
            target_info,
            true,
            true);  
    } catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
    }

    return 0;
}


/**
 * New main function that allows for multiple experiments
 * Usage:
 * ./PFI_Morph help             Print a help message
 * ./PFI_Morph open_mesh        Perform 
 * ./PFI_Morph morph_all        Morph all pairs of models (old morph code)
 */
int main(int argc, const char * argv[]) { 
    // Not enough arguments
    if (argc == 1) {
        std::cout << "Error: Not enough arguments" << std::endl;
        print_help();
        return 1;
    }

    // Compare the first argument with a pre-defined list of commands
    std::string cmd(argv[1]);
    if (cmd == "open_mesh") {
        return load_open_mesh(argc, argv);
    } else if (cmd == "help") {
        print_help();
        return 0;
    } else {
        std::cout << "Error: Unknown command " << cmd << std::endl;
        print_help();
        return 0;
    }
}
