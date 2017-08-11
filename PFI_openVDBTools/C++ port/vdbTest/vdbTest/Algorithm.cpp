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
    for (FloatGrid::ValueAllIter iter = p -> beginValueAll() ; iter; ++iter) {
        cout << iter.getValue() << endl;
    }
}
