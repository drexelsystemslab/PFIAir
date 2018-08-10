
#include <openvdb/openvdb.h>

#include "MorphOperations.h"
#include "MeshOperations.h"
#include "Container.hpp"
#include "GridOperations.h"
#include "CommonOperations.h"
#include "HTMLHelper.h"

#include "UpdtMeshOperations.h"

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
    
    std::string model_names[] = {"cylinder", "hand", "stand", "duck", "mushroom-1", "nail-1", "swan", "pebble", "claw", "container", "winding-wheel", "bird", "man-gun", "tool", "modular-hand", "beast", "hammer", "bishop", "cup", "flask", "head", "queen", "table", "spaceship"};
        
    for(int k = 0; k < sizeof(model_names)/ sizeof(model_names[0]); k++) {
        MorphOperations::Morph morph_obj = MorphOperations::Morph(model_names[k] + ".stl.obj", model_names[k] + ".vdb", model_names[k],
                                                                  "/Volumes/ExtHDD/Jeshur/new/morphs/" + model_names[k] + "/",
                                                                  "original_objs/", "vdbs/");;
        
        
        CommonOperations::makeDirs(morph_obj.curr_path.c_str());
        CommonOperations::makeDirs(morph_obj.obj_path.c_str());
        CommonOperations::makeDirs(morph_obj.vdb_path.c_str());
        
        HTMLHelper::TableRow row1;
        HTMLHelper::TableRow row2;
        
        bool ignore = false;
        double energy1 = 0, energy2 = 0;
        
        std::vector<std::vector<HTMLHelper::TableRow>> obj_pair_vector;
        std::vector<HTMLHelper::TableRow> pair;
        
        if ((dir = opendir ("original_objs")) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) {
                ignore = false;
                for(int j = 0; j < sizeof(morph_obj.ignore_arr)/sizeof(morph_obj.ignore_arr[0]); j++)
                    if(morph_obj.ignore_arr[j] == ent->d_name) ignore = true;
                if(ignore) continue;
                
                if(ent->d_type == DT_REG) {
                    
                    morph_obj.source_grid = nullptr;
                    morph_obj.target_grid = nullptr;
                    
                    row1 = HTMLHelper::TableRow();
                    row2 = HTMLHelper::TableRow();
                    
                    morph_obj.file_name = CommonOperations::getFileNameWithoutExtension(std::string(ent->d_name), ".stl");
                    std::cout << morph_obj.file_name << std::endl;
                    
                    UpdtMeshOperations::doAllMeshOperations(morph_obj.obj_path + morph_obj.curr_obj, "srt1.obj");
                    UpdtMeshOperations::doAllMeshOperations(morph_obj.obj_path + ent->d_name, "srt2.obj");
                    
                    UpdtMeshOperations::convertMeshToVolume("srt1.obj", morph_obj.curr_name, morph_obj.vdb_path, morph_obj.source_nb, morph_obj.voxel_size);
                    UpdtMeshOperations::convertMeshToVolume("srt2.obj", morph_obj.file_name + ".vdb", morph_obj.vdb_path, morph_obj.target_nb, morph_obj.voxel_size);
                    
                    morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.curr_name));
                    morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.file_name + ".vdb"));
                    
                    morph_obj.morph_path = morph_obj.curr_path + morph_obj.curr_name_wo_ext + "-" + morph_obj.file_name;
                    
                    row1.source_name = morph_obj.curr_name_wo_ext;
                    row1.target_name = morph_obj.file_name;
                    
                    energy1 = morph_obj.morphModels(row1);
                    
                    morph_obj.source_grid = nullptr;
                    morph_obj.target_grid = nullptr;
                    
                    UpdtMeshOperations::convertMeshToVolume("srt1.obj", morph_obj.curr_name, morph_obj.vdb_path, morph_obj.target_nb, morph_obj.voxel_size);
                    UpdtMeshOperations::convertMeshToVolume("srt2.obj", morph_obj.file_name + ".vdb", morph_obj.vdb_path, morph_obj.source_nb, morph_obj.voxel_size);
                    
                    morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.file_name + ".vdb"));
                    morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.curr_name));
                    
                    morph_obj.morph_path = morph_obj.curr_path + morph_obj.file_name + "-" + morph_obj.curr_name_wo_ext;
                    
                    row2.source_name = morph_obj.file_name;
                    row2.target_name = morph_obj.curr_name_wo_ext;
                    
                    energy2 = morph_obj.morphModels(row2);
                    row2.mean = (energy1 + energy2) / 2;
                    
                    pair.push_back(row1);
                    pair.push_back(row2);
                    obj_pair_vector.push_back(pair);
                    pair.clear();
                    HTMLHelper::writeReport(obj_pair_vector, morph_obj.curr_name_wo_ext);
                    HTMLHelper::saveObjectState(obj_pair_vector, morph_obj.curr_name_wo_ext + ".json");
                }
            }
            HTMLHelper::writeReport(obj_pair_vector, morph_obj.curr_name_wo_ext);
            HTMLHelper::saveObjectState(obj_pair_vector, morph_obj.curr_name_wo_ext + ".json");
            closedir (dir);
        } else {
            /* could not open directory */
            perror ("");
            return EXIT_FAILURE;
        }
    }
    
    return 0;
}