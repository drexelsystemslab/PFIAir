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
 * Check if a file exists. I use this for
 * caching the centroid calculation
 */
bool file_exists(std::string fname) {
    std::ifstream file(fname);
    if (file)
        return true;
    else
        return false;
}

/**
 * Test of scan-converting a non-watertight model
 */
int load_open_mesh(int argc, const char* argv[]) {
    /*
    if (argc < 6) {
        std::cout << "Missing arguments" << std::endl;
        return 1;
    }

    // Limit memory so my laptop doesn't lock up while testing 
    // existing code with open meshes
    limit_memory(std::atol(argv[2])); 

    // This model is a partial vase.
    const std::string INPUT_OBJ = argv[5]; //"open_mesh_objs/vase_simplified2.obj";
    const std::string OUTPUT_PATH = "output/open_mesh/";
    const std::string OUTPUT_VDB =
        OUTPUT_PATH + "output" + argv[3] + "_" + argv[4] + ".vdb";
    const std::string LS_VDB = OUTPUT_PATH + "level_set.vdb";
    const std::string OUTPUT_FIXED_VDB = OUTPUT_PATH + "fixed.vdb";
    const std::string PREPROCESSED_OBJ = OUTPUT_PATH + "preprocessed.obj";
    CommonOperations::makeDirs(OUTPUT_PATH.c_str());
    const std::string VDB_MAX = OUTPUT_PATH + "max_isoband.vdb";
    const std::string VDB_MID = OUTPUT_PATH + "mid_isoband.vdb";

    const std::string MESH_CENTER_CACHE_FILE = OUTPUT_PATH + "cache_center.txt";
    */

    const std::string INPUT_PATH = "open_mesh_objs/";
    const std::string OUTPUT_PATH = "output/open_mesh/";
    // Original OBJ files
    const std::string SOURCE_OBJ = INPUT_PATH + "source.obj";
    const std::string TARGET_OBJ = INPUT_PATH + "target.obj";

    // After mesh pre-processing.
    const std::string SOURCE_OBJ_PROCESSED = 
        OUTPUT_PATH + "source_processed.obj";
    const std::string TARGET_OBJ_PROCESSED =
        OUTPUT_PATH + "target_processed.obj";

    // Limit memory usage to 1 GB as a safety precaution. I don't want to 
    // lock up my laptop again.
    limit_memory(1000);


    // New style of pre-processing meshes with a much simpler interface.
    std::cout << "Process new-style mesh" << std::endl;
    Mesh mesh(SOURCE_OBJ, true);
    mesh.preprocess_mesh();
    mesh.save_obj(OUTPUT_PATH + "new_style_mesh.obj");
    std::cout << "Done!" << std::endl;

    return 0;

    /**
     * Pre-process meshes. The results are saved to a file for two reasons:
     * 1. I can inspect the results
     * 2. I can cache the results
     *
     * To force a pre-processing, delete the processed OBJS in the output
     * directory.
     * 
     * NOTE: if eccentricity is used to find the mesh center,
     * this runs in O(V^3) time so only is feasible for very small meshes.
     */
    if (!file_exists(SOURCE_OBJ_PROCESSED)) {
        std::cout << "Pre-processing source object (SLOW)" << std::endl;
        UpdtMeshOperations::doAllMeshOperations(
            OUTPUT_PATH,
            SOURCE_OBJ,
            SOURCE_OBJ_PROCESSED,
            // Explicitly specifiy that thi is an open mesh
            // TODO: Is there an easy way to check if a mesh is closed?
            false);
    }
    if (!file_exists(TARGET_OBJ_PROCESSED)) {
        std::cout << "Pre-processing target object (SLOW)" << std::endl;
        UpdtMeshOperations::doAllMeshOperations(
            OUTPUT_PATH,
            TARGET_OBJ,
            TARGET_OBJ_PROCESSED,
            false);
    }

    // Create an object for morphing.
    MorphOperations::Morph morph_obj = MorphOperations::Morph(SOURCE_OBJ);
        
    // Convert meshes to volumes
    const std::string SOURCE_VDB = "source.vdb";
    const std::string TARGET_VDB = "target.vdb";
    const double BANDWIDTH = 3.0;
    const double VOXEL_SIZE = 0.05;

    std::cout << "Converting source object to volume (SLOW)" << std::endl; 
    UpdtMeshOperations::convertMeshToVolume(
        SOURCE_OBJ_PROCESSED,
        SOURCE_VDB,
        OUTPUT_PATH,
        BANDWIDTH,
        VOXEL_SIZE,
        false);
    std::cout << "Converting target object to volume (SLOW)" << std::endl; 
    UpdtMeshOperations::convertMeshToVolume(
        TARGET_OBJ_PROCESSED,
        TARGET_VDB,
        OUTPUT_PATH,
        BANDWIDTH,
        VOXEL_SIZE,
        false);

    std::cout << "Reading VDB files" << std::endl;
    morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
        GridOperations::readFile(OUTPUT_PATH + SOURCE_VDB));
    morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
        GridOperations::readFile(OUTPUT_PATH + TARGET_VDB));

    // Set the morph output directory
    morph_obj.morph_path = OUTPUT_PATH + "source-target";
                
    // Generate a Table Row
    HTMLHelper::TableRow row;    
    std::cout << "Morphing models" << std::endl;
    double energy = morph_obj.morphModels(row);
    std::cout << "Energy: " << energy << std::endl;

    /*
    PFIAir::Container model = PFIAir::Container();

    std::cout << "Loading Open Mesh" << std::endl;
    model.loadMeshModel(INPUT_OBJ);

    openvdb::Vec3s center = model.computeMeshCenter();
    std::cout << "Center " << center << std::endl;

    // IMPORTANT: Container.computeMeshCenter does not work with open meshes!
    // It consumes all memory
    //model.computeMeshCenter(); 

    double voxel_size = std::stod(argv[3]);
    std::cout << "voxel size" << voxel_size;
    model.setScale(openvdb::Vec3d(voxel_size));

    // Convert to unsigned distance field
    std::cout << "Converting to VDB" << std::endl; 
    double bandwidth = std::stod(argv[4]);
    std::cout << "bandwidth" << bandwidth;
    openvdb::FloatGrid::Ptr field = 
        model.getUnsignedDistanceField(bandwidth);

    model.exportModel(OUTPUT_VDB, field);

    std::cout << "Converted " << INPUT_OBJ << " -> " << OUTPUT_VDB << std::endl;

    // Convert distance field to level set
    dist_field_to_level_set(field, bandwidth, voxel_size);
    model.exportModel(LS_VDB, field);

    // Exploring the range of values in the distance field
    // since the docs do not go into depth
    int count = 0;
    double min = 100000.0;
    double max = -100000.0;
    double total = 0.0;
    for (openvdb::FloatGrid::ValueOnIter iter = field->beginValueOn(); iter; ++iter) {
        if (*iter < min)
            min = *iter;
        if (*iter > max)
            max = *iter;
        total += *iter;
        count++;
        //std::cout << iter.getCoord() << " = " << *iter << std::endl; 
    }
    double avg = total / count;
    double midpoint = (min + max) / 2.0;
    std::cout << "total count avg midpoint" << std::endl;
    std::cout << total << " " << count << " " << avg << " " << midpoint << std::endl;
    std::cout << "min max" << std::endl;
    std::cout << min << " " << max << std::endl;

    // Attempt to convert unsigned distance field -> mesh

    // Vec3s = single-precision float
    std::vector<openvdb::Vec3s> points;
    std::vector<openvdb::Vec3I> triangles;
    std::vector<openvdb::Vec4I> quads;

    std::cout << "num_points num_triangles num_quads" << std::endl;

    // volumeToMesh has a isovalue 
    // default of 0.0 generates no quads
    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*field, points, triangles, quads);
    std::cout << "isovalue = 0.0" << std::endl;
    std::cout << points.size() << " " << triangles.size() << " " << quads.size() << std::endl;

    // Try the minimum iso value
    // this generates no quads
    points.clear();
    triangles.clear();
    quads.clear();
    std::cout << "isovalue = min" << std::endl;
    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*field, points, triangles, quads, min);
    std::cout << points.size() << " " << triangles.size() << " " << quads.size() << std::endl;

    // Try the maximum iso value
    // This generates the most quads
    points.clear();
    triangles.clear();
    quads.clear();
    std::cout << "isovalue = max" << std::endl;
    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*field, points, triangles, quads, max);
    std::cout << points.size() << " " << triangles.size() << " " << quads.size() << std::endl;

    // Try the midpoint of max and min iso value
    points.clear();
    triangles.clear();
    quads.clear();
    std::cout << "isovalue = (min + max) / 2" << std::endl;
    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*field, points, triangles, quads, midpoint);
    std::cout << points.size() << " " << triangles.size() << " " << quads.size() << std::endl;
    */
    
    // And now we can create a level set
    // When this runs on the max or midpoint, I do get a level set, but it
    // is a rectangular prism... not sure why, I think it has to do with
    // scaling.
    /* 
    openvdb::math::Transform xform;
    openvdb::FloatGrid::Ptr fixed =
        openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(
            xform, points, triangles, quads);

    model.exportModel(VDB_MID, fixed);
    */

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
        std::cout << "Open Mesh Experiment" << std::endl;
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
