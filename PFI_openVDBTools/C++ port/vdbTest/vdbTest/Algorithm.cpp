//
//  Algorithm.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 8/11/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include "Algorithm.hpp"

using namespace std;

bool PFIAir::algorithme::inflate(const float offset, FloatGrid::Ptr) {
    return true;
}

void PFIAir::algorithme::printValues(FloatGrid::Ptr p) {
//    for (FloatGrid::ValueAllIter iter = p -> beginValueAll() ; iter; ++iter) {
//        cout << iter.getValue() << endl;
//    }
    
    for (FloatGrid::ValueOnCIter iter = p -> cbeginValueOn() ; iter; ++iter) {
        cout << iter.getValue() << endl;
    }
}

void PFIAir::algorithme::changeActiveVoxelValues(FloatGrid::Ptr p, float offset) {
    for (FloatGrid::ValueOnIter iter = p -> beginValueOn() ; iter; ++iter) {
        float dist = iter.getValue();
        iter.setValue(dist - offset);
    }
}

// write loops to check signed vs unsigned sizes
// interseting voxel or bounding box
// corner 
