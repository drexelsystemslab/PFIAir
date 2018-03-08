#include <openvdb/openvdb.h>

#include "MorphOperations.h"
#include "MeshOperations.h"
#include "Container.hpp"
#include "GridOperations.h"
#include "CommonOperations.h"
#include "HTMLHelper.h"

int main()
{
    openvdb::initialize();
    //openvdb::FloatGrid::Ptr source_grid1 = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile("cylinder.vdb"));
    

//    std::vector<std::vector<HTMLHelper::TableRow>> pairs = HTMLHelper::getObjectFromState("state.json");
//    HTMLHelper::writeReport(pairs, "cylinder");
//    return 0;
    
    DIR *dir;
    struct dirent *ent;
    
    MorphOperations::Morph morph_obj = MorphOperations::Morph("cylinder.stl.obj", "cylinder.vdb", "cylinder",
                                                              "/Volumes/ExtHDD/Jeshur/morphs/cylinder/after-fix/",
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
                
                MeshOperations::doAllMeshOperations(morph_obj.obj_path + morph_obj.curr_obj, "srt1.obj");
                MeshOperations::doAllMeshOperations(morph_obj.obj_path + ent->d_name, "srt2.obj");
                
                MeshOperations::convertMeshToVolume("srt1.obj", morph_obj.curr_name, morph_obj.vdb_path, morph_obj.source_nb, morph_obj.voxel_size);
                MeshOperations::convertMeshToVolume("srt2.obj", morph_obj.file_name + ".vdb", morph_obj.vdb_path, morph_obj.target_nb, morph_obj.voxel_size);
                
                morph_obj.source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.curr_name));
                morph_obj.target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(morph_obj.vdb_path + morph_obj.file_name + ".vdb"));
                
                morph_obj.morph_path = morph_obj.curr_path + morph_obj.curr_name_wo_ext + "-" + morph_obj.file_name;
                
                row1.source_name = morph_obj.curr_name_wo_ext;
                row1.target_name = morph_obj.file_name;
                
                energy1 = morph_obj.morphModels(row1);
                
                morph_obj.source_grid = nullptr;
                morph_obj.target_grid = nullptr;
                
                MeshOperations::convertMeshToVolume("srt1.obj", morph_obj.curr_name, morph_obj.vdb_path, morph_obj.target_nb, morph_obj.voxel_size);
                MeshOperations::convertMeshToVolume("srt2.obj", morph_obj.file_name + ".vdb", morph_obj.vdb_path, morph_obj.source_nb, morph_obj.voxel_size);
                
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
            }
        }
        HTMLHelper::writeReport(obj_pair_vector, morph_obj.curr_name_wo_ext);
        HTMLHelper::saveObjectState(obj_pair_vector, "state.json");
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }
    return 0;
    
    
}