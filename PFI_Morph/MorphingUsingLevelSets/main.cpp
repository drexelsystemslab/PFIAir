#include <iostream>

#include <sys/time.h>
#include <sys/resource.h>

#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>

#include "MorphOperations.h"
#include "MeshOperations.h"
#include "Container.hpp"
#include "GridOperations.h"
#include "CommonOperations.h"
#include "HTMLHelper.h"

#include "UpdtMeshOperations.h"
#include "Mesh.h"
#include "Timer.h"
#include "Morph.h"

#include "ReportGenerator.h"

//const std::string MORPH_OUTPUT_DIR = "/Volumes/ExtHDD/Jeshur/new/morphs/";
const std::string OUTPUT_DIR = "output/";
const std::string INPUT_DIR = "original_objs/";
const std::string TEMP_OBJ1 = OUTPUT_DIR + "srt1.obj";
const std::string TEMP_OBJ2 = OUTPUT_DIR + "srt2.obj";
const std::string VDB_DIR = OUTPUT_DIR + "vdbs";

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


/**
Main from mac_tools
int main(int argc, const char * argv[])
{
    using namespace openvdb;
    
//    PFIAir::Container model = PFIAir::Container();
//
//    model.loadMeshModel(argv[1]);
//    model.computeMeshCenter();
    
    MeshOperations::doAllMeshOperations(argv[1], argv[2]);
    return EXIT_SUCCESS;
    
    
    openvdb::initialize(); 
    
//    openvdb::FloatGrid::Ptr source_grid1 = CommonOperations::getSphereVolume(1.5, 0.025, 3);
//    openvdb::FloatGrid::Ptr target_grid1 = CommonOperations:: getPlatonicVolume(6, 3, 0.025, 10);
//    std::string table1 = "";
    
//    MorphOperations::morphModels(source_grid1, target_grid1, 0.1, "test", table1);
//    return 0;
//    MeshOperations::performPCAAndDrawAxis("original_objs/bird.stl.obj", "test.obj");
//    MeshOperations::doAllMeshOperations("original_objs/beast.stl.obj", "test.obj");
//    MeshOperations::convertMeshToVolume("test.obj", "test.vdb", "vdbs/", 3, 0.01);
//    return 0;
}
*/

/**
 * This was the old main() from the morph branch
 */
int morph_all()
{
    openvdb::initialize();

//    GridOperations::doMorphologicalOpening(source_grid1);
//    GridOperations::writeToFile("test.vdb", source_grid1);
    
//    MeshOperations::doAllMeshOperations("new_objs/cup.obj", "srt_cup.obj");
//    MeshOperations::doAllMeshOperations("new_objs/flask.obj", "srt_flask.obj");
//    MeshOperations::doAllMeshOperations("new_objs/hammer.obj", "srt_hammer.obj");
//    MeshOperations::doAllMeshOperations("new_objs/head.obj", "srt_head.obj");
//    MeshOperations::doAllMeshOperations("new_objs/spaceship.obj", "srt_spaceship.obj");
//    MeshOperations::doAllMeshOperations("new_objs/table.obj", "srt_table.obj");
    
    //UpdtMeshOperations::performActionForAllObjs();
    //UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-1.stl.obj", "test/mushroom-1.stl.obj");
    //UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-2.stl.obj", "test/mushroom-2.stl.obj");
    //UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-3.stl.obj", "test/mushroom-3.stl.obj");
    //UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-4.stl.obj", "test/mushroom-4.stl.obj");
    //UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-5.stl.obj", "test/mushroom-5.stl.obj");
    //UpdtMeshOperations::doAllMeshOperations("original_objs/pendulum.stl.obj", "test/pendulum.stl.obj");
    //return 0;
    
    DIR *dir;
    struct dirent *ent;
    
    // List of objects
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
    
    CommonOperations::makeDirs(OUTPUT_DIR.c_str());
        
    // For every model
    for(int k = 0; k < sizeof(model_names)/ sizeof(model_names[0]); k++) {
        // Create a data structure for morphing
        MorphOperations::Morph morph_obj = MorphOperations::Morph(
            // obj file
            model_names[k] + ".stl.obj", 
            // VDB file
            model_names[k] + ".vdb", 
            // name without extension.
            model_names[k],
            //path for the models
            OUTPUT_DIR + model_names[k] + "/",
            // Object files go here
            INPUT_DIR, 
            // VDB files go here
            VDB_DIR);
        
        
        // Make sure the paths exist
        CommonOperations::makeDirs(morph_obj.curr_path.c_str());
        CommonOperations::makeDirs(morph_obj.obj_path.c_str());
        CommonOperations::makeDirs(morph_obj.vdb_path.c_str());
        
        // This data structure stores stats about the current morph
        // row 1 is from A -> B
        // row 2 is from B -> A
        HTMLHelper::TableRow row1;
        HTMLHelper::TableRow row2;
        
        bool ignore = false;
        double energy1 = 0, energy2 = 0;
        
        // Vector of stat row pairs.
        // TODO: maybe use  a vector of std::pair<TableRow, TableRow>
        // to be clearer about intent?
        std::vector<std::vector<HTMLHelper::TableRow>> obj_pair_vector;
        std::vector<HTMLHelper::TableRow> pair;
        
        // Iterate over the OBJ files
        if ((dir = opendir(INPUT_DIR.c_str())) != NULL) {
            // print all the files and directories within directory
            while ((ent = readdir(dir)) != NULL) {
                ignore = false;

                // Check if this 
                // TODO: This should be hhandled inside the morph
                // object. Also use STL data structures for clearer code
                int ignore_arr_size = (
                    sizeof(morph_obj.ignore_arr)
                    /sizeof(morph_obj.ignore_arr[0]));
                for(int j = 0; j < ignore_arr_size; j++)
                    if(morph_obj.ignore_arr[j] == ent->d_name) ignore = true;
                if(ignore) {
                    std::cout << "Ignoring file " << ent->d_name << std::endl;
                    continue;
                }
                
                // If we find a regular file
                if(ent->d_type == DT_REG) {
                    std::cout << "Examining file " << ent->d_name << std::endl;

                    // Source and target VDB grids
                    morph_obj.source_grid = nullptr;
                    morph_obj.target_grid = nullptr;
                    
                    // Create empty table rows
                    row1 = HTMLHelper::TableRow();
                    row2 = HTMLHelper::TableRow();
                    
                    // Extract the morph filename from the 
                    morph_obj.file_name = CommonOperations::getFileNameWithoutExtension(
                        std::string(ent->d_name), ".stl");
                    std::cout << morph_obj.file_name << std::endl;
                    
                    // Orient the meshes and store them to temporary .obj files
                    std::cout << "Orienting Meshes" << std::endl;
                    UpdtMeshOperations::doAllMeshOperations(
                        OUTPUT_DIR, 
                        morph_obj.obj_path + morph_obj.curr_obj, 
                        TEMP_OBJ1);
                    UpdtMeshOperations::doAllMeshOperations(
                        OUTPUT_DIR, 
                        morph_obj.obj_path + ent->d_name, 
                        TEMP_OBJ2);
                    
                    // Convert the modified meshes into VDB volumes
                    // and store them in vdb files
                    std::cout << "Converting Meshes -> volumes" << std::endl;
                    UpdtMeshOperations::convertMeshToVolume(
                        TEMP_OBJ1, 
                        morph_obj.curr_name, 
                        morph_obj.vdb_path, 
                        morph_obj.source_nb, 
                        morph_obj.voxel_size);
                    UpdtMeshOperations::convertMeshToVolume(
                        TEMP_OBJ2, 
                        morph_obj.file_name + ".vdb", 
                        morph_obj.vdb_path, 
                        morph_obj.target_nb, 
                        morph_obj.voxel_size);
                    
                    // Read in the files
                    std::cout << "Reading VDB files" << std::endl;
                    morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
                        GridOperations::readFile(
                            morph_obj.vdb_path + morph_obj.curr_name));
                    morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
                        GridOperations::readFile(
                        morph_obj.vdb_path + morph_obj.file_name + ".vdb"));
                    
                    morph_obj.morph_path = (
                        morph_obj.curr_path 
                        + morph_obj.curr_name_wo_ext 
                        + "-" 
                        + morph_obj.file_name);
                    std::cout << "Morph Path" << std::endl;
                    
                    // Store the model names in the stat data structures
                    row1.source_name = morph_obj.curr_name_wo_ext;
                    row1.target_name = morph_obj.file_name;
                    
                    std::cout << "Morphing models" << std::endl;
                    energy1 = morph_obj.morphModels(row1);
                    
                    // Clear the grid points
                    morph_obj.source_grid = nullptr;
                    morph_obj.target_grid = nullptr;
                    
                    // TODO: Why is this done twice?
                    std::cout << "Converting Mesh -> Volumes (Again)" << std::endl; 
                    UpdtMeshOperations::convertMeshToVolume(
                        TEMP_OBJ1, 
                        morph_obj.curr_name, 
                        morph_obj.vdb_path, 
                        morph_obj.target_nb, 
                        morph_obj.voxel_size);
                    UpdtMeshOperations::convertMeshToVolume(
                        TEMP_OBJ2, 
                        morph_obj.file_name + ".vdb", 
                        morph_obj.vdb_path, 
                        morph_obj.source_nb, 
                        morph_obj.voxel_size);
                    
                    // Again, read in the models.
                    std::cout << "Reading VDB files (again)" << std::endl;
                    morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
                        GridOperations::readFile(morph_obj.vdb_path + morph_obj.file_name + ".vdb"));
                    morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
                        GridOperations::readFile(morph_obj.vdb_path + morph_obj.curr_name));
                    
                    morph_obj.morph_path = (
                        morph_obj.curr_path 
                        + morph_obj.file_name 
                        + "-" 
                        + morph_obj.curr_name_wo_ext);
                    std::cout << "Morph path (again): " << morph_obj.morph_path << std::endl;
                    
                    row2.source_name = morph_obj.file_name;
                    row2.target_name = morph_obj.curr_name_wo_ext;
                    
                    // Compute energy the other way
                    energy2 = morph_obj.morphModels(row2);
                    row2.mean = (energy1 + energy2) / 2;
                    
                    // Store the table row in the vector
                    pair.push_back(row1);
                    pair.push_back(row2);
                    obj_pair_vector.push_back(pair);
                    pair.clear();
                    
                    // Generate the report
                    std::cout << "Generating HTML file" << std::endl;
                    HTMLHelper::writeReport(obj_pair_vector, morph_obj.curr_name_wo_ext);
                    HTMLHelper::saveObjectState(obj_pair_vector, morph_obj.curr_name_wo_ext + ".json");

                    // TODO: Remove this
                    // It takes a few minutes to morph a single pair of models
                    // Uncomment this line to only run the first pair
                    break;
                }
            }
            // Generate a last report
            // TODO: what does this last report do?
            std::cout << "writing report for " << morph_obj.curr_name_wo_ext << std::endl;
            HTMLHelper::writeReport(obj_pair_vector, morph_obj.curr_name_wo_ext);
            HTMLHelper::saveObjectState(obj_pair_vector, morph_obj.curr_name_wo_ext + ".json");
            closedir (dir);
        } else {
            // could not open directory
            perror("Could not open directory");
            return EXIT_FAILURE;
        }
    }
    
    return 0;
}

void print_help() {
    std::cout << "PFI_Morph: similartiy using level set morphing" << std::endl
        << "Usage: PFI_Morph [cmd]" << std::endl
        << std::endl
        << "Commands:" << std::endl
        << "open_mesh mem_limit voxel_size bandwidth\tRun experiment for morphing open meshes" << std::endl
        << "morph_all\tMorph all pairs of models and generate reports" 
        << std::endl
        << "help\t\tPrint this help message and quit" << std::endl;
}


/**
 * Test of scan-converting a non-watertight model
 */
int load_open_mesh(int argc, const char* argv[]) {
    const std::string INPUT_PATH = "open_mesh_objs/";
    const std::string OUTPUT_PATH = "output/open_mesh/";
    // Original OBJ files
    const std::string SOURCE_OBJ = INPUT_PATH + "source.obj";
    const std::string TARGET_OBJ = INPUT_PATH + "target.obj";

    CommonOperations::makeDirs(OUTPUT_DIR.c_str());

    // After mesh pre-processing.
    const std::string SOURCE_OBJ_PROCESSED = 
        OUTPUT_PATH + "source_processed.obj";
    const std::string TARGET_OBJ_PROCESSED =
        OUTPUT_PATH + "target_processed.obj";

    // After converting to VDBs
    const std::string SOURCE_VDB_PROCESSED = 
        OUTPUT_PATH + "source_processed.vdb";
    const std::string TARGET_VDB_PROCESSED =
        OUTPUT_PATH + "target_processed.vdb";

    /*
    // Test the report generator
    const std::string REPORT_FILE = "Reports/open_mesh.html";
    ReportGenerator report(REPORT_FILE);

    MorphStats forwards;
    forwards.set_names("source", "target");
    forwards.count_surface_voxels(3000, 4000);
    forwards.increment_time();
    forwards.update_energy(30, 10);
    forwards.add_cfl_iterations(50);
    forwards.update_max_curvature(3);

    forwards.increment_time();
    forwards.update_energy(40, 50);
    forwards.add_cfl_iterations(20);
    forwards.update_max_curvature(2);

    forwards.increment_time();
    forwards.update_energy(10, 10);
    forwards.add_cfl_iterations(15);
    forwards.update_max_curvature(5);

    forwards.finalize_stats();

    MorphStats backwards;
    report.add_row(forwards, backwards);
    report.write_report();
    */

    // Limit memory usage to 1 GB as a safety precaution. I don't want to 
    // lock up my laptop again.
    limit_memory(1000);

    // Read in the two open meshes and preprocess them ====================

    Timer time_overall("Open Mesh Experiment");
    time_overall.start();

    Timer time_preprocess("Preprocessing meshes");
    time_preprocess.start();

    std::cout << "Preprocess source mesh" << std::endl;
    // boolean flag is to mark this as an open mesh
    Mesh source_mesh(SOURCE_OBJ, true);
    source_mesh.preprocess_mesh();

    std::cout << "Preprocess target mesh" << std::endl;
    Mesh target_mesh(TARGET_OBJ, true);
    target_mesh.preprocess_mesh();

    time_preprocess.stop();

    // Optional: Save processed meshes ==================================

    Timer time_save_obj("(Optional) Saving OBJ files");
    time_save_obj.start();

    std::cout << "Saving source mesh to " << SOURCE_OBJ_PROCESSED << std::endl;
    source_mesh.save_obj(SOURCE_OBJ_PROCESSED); 

    std::cout << "Saving target mesh to " << TARGET_OBJ_PROCESSED << std::endl;
    target_mesh.save_obj(TARGET_OBJ_PROCESSED);

    time_save_obj.stop();

    // Convert to level sets ===========================================

    Timer time_convert("Converting meshes -> level sets");
    time_convert.start();

    std::cout << "Converting source mesh to level set" << std::endl;
    LevelSet source_ls = source_mesh.to_level_set();

    std::cout << "Converting target mesh to level set" << std::endl;
    LevelSet target_ls = target_mesh.to_level_set();

    time_convert.stop();

    // Optional: Save level sets ======================================

    Timer time_save_vdb("(Optional) Saving VDBs");
    time_save_vdb.start();

    std::cout << "Saving source mesh to " << SOURCE_VDB_PROCESSED << std::endl;
    source_ls.save(SOURCE_VDB_PROCESSED); 

    std::cout << "Saving target mesh to " << TARGET_VDB_PROCESSED << std::endl;
    target_ls.save(TARGET_VDB_PROCESSED);

    time_save_vdb.stop();

    // Morph the two models ============================================

    Timer time_morph("Morphing Models");
    time_morph.start();

    Morph morph_obj;
    std::string morph_dir = OUTPUT_PATH + "source-target";

    Timer timer_fwd("Morph source -> target");
    timer_fwd.start();
    MorphStats fwd_stats = morph_obj.morph(
        source_ls, target_ls, "source-target");
    timer_fwd.stop();

    Timer timer_bwd("Morph source <- target");
    timer_bwd.start();
    MorphStats bwd_stats = morph_obj.morph(
        target_ls, source_ls, "target-source");
    timer_bwd.stop();

    time_morph.stop();

    time_overall.stop();

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
    } else if (cmd == "morph_all") {
        std::cout << "Morph all Meshes:" << std::endl;
        return morph_all();
    } else if (cmd == "help") {
        print_help();
        return 0;
    } else {
        std::cout << "Error: Unknown command " << cmd << std::endl;
        print_help();
        return 0;
    }
}
