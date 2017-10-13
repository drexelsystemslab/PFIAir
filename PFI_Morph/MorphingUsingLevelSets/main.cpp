#include <openvdb/openvdb.h>
//#include "Tests.h"

#include "MeshOperations.h"


int main()
{
    openvdb::initialize();
    
//    openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile("sphere.vdb"));
//    GridOperations::writeToFile("read_sphere.vdb", source_grid);
    
    
//    Tests::performMorphPermutation();
    
    std::string filepath1 = "/Users/jpt54/Documents/Projects/MorphingUsingLevelSets/DerivedData/MorphingUsingLevelSets/Build/Products/Debug/hand.obj";
    std::string filepath2 = "/Users/jpt54/Documents/Projects/MorphingUsingLevelSets/DerivedData/MorphingUsingLevelSets/Build/Products/Debug/tool.obj";
    
    std::vector<std::vector<std::string>> v_list;
    std::vector<std::vector<std::string>> vn_list;
    std::vector<std::vector<std::string>> f_list1, f_list2;
    std::vector<Eigen::Matrix<double, 1, 4>> vertices1, vertices2;
    Eigen::Matrix4d rot_mat1, rot_mat2;
    std::vector<double> axis_lengths1, axis_lengths2;
    
    MeshOperations::readOBJ(filepath1, v_list, vn_list, f_list1);
    MeshOperations::performPCA(v_list, vertices1, rot_mat1);
    
    for(int i = 0; i < vertices1.size(); i++)
        vertices1[i] *= rot_mat1;
    
    
    MeshOperations::calcBoundingBox(vertices1, axis_lengths1);
    
    v_list.clear();
    vn_list.clear();
    
    
    MeshOperations::readOBJ(filepath2, v_list, vn_list, f_list2);
    MeshOperations::performPCA(v_list, vertices2, rot_mat2);
    
    for(int i = 0; i < vertices2.size(); i++)
        vertices2[i] *= rot_mat2;
    
    
    MeshOperations::calcBoundingBox(vertices2, axis_lengths2);
    
    MeshOperations::adjustScale(vertices1, vertices2, axis_lengths1[2], axis_lengths2[2]);
    
    axis_lengths1.clear();
    axis_lengths2.clear();
    MeshOperations::calcBoundingBox(vertices1, axis_lengths1);
    MeshOperations::calcBoundingBox(vertices2, axis_lengths2);
    
    MeshOperations::translateCentroidToOrigin(vertices1, vertices2);
    
    MeshOperations::writeOBJ("rotate1.obj", vertices1, f_list1);
    MeshOperations::writeOBJ("rotate2.obj", vertices2, f_list2);
    
    return 0;
}




