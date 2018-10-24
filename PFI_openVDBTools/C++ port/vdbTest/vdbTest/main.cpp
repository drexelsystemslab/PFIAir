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
    
    //1
//    PFIAir::Container model = PFIAir::Container();
//
//    model.loadMeshModel(argv[1]);
//    model.computeMeshCenter();
//
//
//    return EXIT_SUCCESS;
    
    
    //
//    float edgeLen = model.computeAverageEdgeLength() / 10;
//
//    model.setScale(Vec3d(edgeLen,edgeLen,edgeLen));
//
//    auto cube = model.getUnsignedDistanceField(3);
//
//    PFIAir::algorithme::changeActiveVoxelValues(cube, 0.5);
    
    //scale
//    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
//    openvdb::math::Transform::Ptr linearTransform =
//    openvdb::math::Transform::createLinearTransform(mat);
//
//    float scaleLen = 1 / model.computeAverageEdgeLength();
//    cube -> transform().postScale(Vec3d(scaleLen,scaleLen,scaleLen));
//    linearTransform->preScale(Vec3d(scaleLen,scaleLen,scaleLen));
//    
//    cube->setTransform(linearTransform);
    
    
    //end
    
    //model.exportModel(argv[2], cube);
    //PFIAir::algorithme::printValues(cube);
    
    //std::cout << model.computeAverageEdgeLength() << std::endl;
    
    //tools::LevelSetFilter<FloatGrid> tracker = tools::LevelSetFilter<FloatGrid>(cube);

    
    return 0;
}

// Unsigned:
// all voxels: 323520
// on voxels: 7472

// Signed:
// all voxels: 323520
// on voxels: 7472
