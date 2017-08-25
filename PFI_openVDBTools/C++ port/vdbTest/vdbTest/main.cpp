//
//  main.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include <iostream>
#include "Container.hpp"
#include "Algorithm.hpp"

int main(int argc, const char * argv[]) {
    using namespace openvdb;
    
    PFIAir::Container model = PFIAir::Container(Vec3d(0.1,0.1,0.1));

    model.loadMeshModel(argv[1]);
    
    auto cube = model.getWaterTightLevelSetWithBandWidth(3);
    
    //PFIAir::algorithme::changeActiveVoxelValues(cube, 0.5);
    
    //manager.exportModel(argv[2], cube);
    //PFIAir::algorithme::printValues(cube);
    
    std::cout << model.computeAverageEdgeLength() << std::endl;
    
    return 0;
}

// Unsigned:
// all voxels: 323520
// on voxels: 7472

// Signed:
// all voxels: 323520
// on voxels: 7472
