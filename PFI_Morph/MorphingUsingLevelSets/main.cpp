
#include <openvdb/openvdb.h>

#include "MorphOperations.h"
#include "MeshOperations.h"
#include "Container.hpp"
#include "GridOperations.h"
#include "CommonOperations.h"
#include "HTMLHelper.h"

#include "UpdtMeshOperations.h"

//const std::string MORPH_OUTPUT_DIR = "/Volumes/ExtHDD/Jeshur/new/morphs/";
const std::string OUTPUT_DIR = "output/";
const std::string INPUT_DIR = "original_objs/";
const std::string TEMP_OBJ1 = OUTPUT_DIR + "srt1.obj";
const std::string TEMP_OBJ2 = OUTPUT_DIR + "srt2.obj";
const std::string VDB_DIR = OUTPUT_DIR + "vdbs";

int main()
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
    /*UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-1.stl.obj", "test/mushroom-1.stl.obj");
    UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-2.stl.obj", "test/mushroom-2.stl.obj");
    UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-3.stl.obj", "test/mushroom-3.stl.obj");
    UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-4.stl.obj", "test/mushroom-4.stl.obj");
    UpdtMeshOperations::doAllMeshOperations("original_objs/mushroom-5.stl.obj", "test/mushroom-5.stl.obj");
    UpdtMeshOperations::doAllMeshOperations("original_objs/pendulum.stl.obj", "test/pendulum.stl.obj");
    return 0;*/
    
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
            VDB_DIR);;
        
        
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
            /* print all the files and directories within directory */
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
            /* could not open directory */
            perror("Could not open directory");
            return EXIT_FAILURE;
        }
    }
    
    return 0;
}
