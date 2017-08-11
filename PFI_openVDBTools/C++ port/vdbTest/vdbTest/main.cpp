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
    
    PFIAir::Container manager = PFIAir::Container(Vec3d(0.1,0.1,0.1));

    manager.loadMeshModel(argv[1]);
    auto cube = manager.getWaterTightLevelSet();
    manager.exportModel(argv[2], cube);
    
    //PFIAir::algorithme::printValues(cube);
    
    return 0;
}
