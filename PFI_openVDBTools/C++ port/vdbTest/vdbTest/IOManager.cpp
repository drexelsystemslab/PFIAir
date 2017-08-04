//
//  IOManager.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include "IOManager.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <openvdb/openvdb.h>
#include <openvdb/points/PointConversion.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/MeshToVolume.h>

IOManager::IOManager() {
    
}

openvdb::GridBase::Ptr IOManager::import_model(const std::string file_name) {
    openvdb::io::File file(file_name);
    
    file.open();
    
    openvdb::GridBase::Ptr _grid;
    
    for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter) {
        _grid = file.readGrid(nameIter.gridName());
        // Read in only the grid we are interested in.
        //        if (nameIter.gridName() == "grid_name") {
        //            baseGrid_sphere = file2.readGrid(nameIter.gridName());
        //        } else {
        //            display("Skipping grid", nameIter.gridName());
        //        }
    }
    file.close();
    
    return _grid;
}

auto points = std::vector<openvdb::Vec3s>();
auto indicesTri = std::vector<openvdb::Vec3I>();
auto indicesQuad = std::vector<openvdb::Vec4I>();


// code partially from http://www.cplusplus.com/doc/tutorial/files/
// and https://stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string
void flush() {
}


void objFileImport() {
    std::string line;
    std::ifstream myfile ("man.obj");
    
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            
            std::istringstream iss(line);
            std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}};
            
            if (tokens.size() > 0) {
                // vertices
                if (!tokens[0].compare("v")) {
                    //std::cout << tokens[0] << tokens.size() << '\n';
                    assert(tokens.size() == 4);
                    
                    points.push_back(openvdb::Vec3s(std::stof(tokens[1]),std::stof(tokens[2]),std::stof(tokens[3])));
                }
                
                // facets
                if (!tokens[0].compare("f")) {
                    //std::cout << tokens[0] << tokens.size()  << '\n';
                    assert(tokens.size() == 4 || tokens.size() == 5 );
                    
                    if (tokens.size() == 4) {
                        indicesTri.push_back(openvdb::Vec3I(std::stoi(tokens[1]),std::stoi(tokens[2]),std::stoi(tokens[3])));
                    } else {
                        indicesQuad.push_back(openvdb::Vec4I(std::stoi(tokens[1]),std::stoi(tokens[2]),std::stoi(tokens[3]),std::stoi(tokens[4])));
                    }
                }
            }
        }
        myfile.close();
    }
    
    else std::cout << "Unable to open file";
}

bool IOManager::import_stl(const std::string file_name) {
    
    objFileImport();
    
    openvdb::initialize();
    
    auto transform = openvdb::math::Transform();
    
    auto levelSet = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(transform, points, indicesTri, indicesQuad);

    
//    openvdb::io::File file("myMan.vdb");
//
//    openvdb::GridPtrVec grids;
//    grids.push_back(levelSet);
//    // Write out the contents of the container.
//    file.write(grids);
//    file.close();
    
//    openvdb::initialize();
//    // Create a FloatGrid and populate it with a narrow-band
//    // signed distance field of a sphere.
//    openvdb::FloatGrid::Ptr grid =
//    openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
//                                                             /*radius=*/50.0, /*center=*/openvdb::Vec3f(1.5, 2, 3),
//                                                             /*voxel size=*/0.5, /*width=*/4.0);
//    // Associate some metadata with the grid.
//    grid->insertMeta("radius", openvdb::FloatMetadata(50.0));
//    // Name the grid "LevelSetSphere".
//    grid->setName("LevelSetSphere");
//    // Create a VDB file object.
//    openvdb::io::File file("mygrids.vdb");
//    // Add the grid pointer to a container.
//    openvdb::GridPtrVec grids;
//    grids.push_back(grid);
//    // Write out the contents of the container.
//    file.write(grids);
//    file.close();
    return true;
}
