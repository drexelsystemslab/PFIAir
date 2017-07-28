//
//  main.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include <iostream>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetMorph.h>
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/math/Maps.h>
#include <openvdb/math/Operators.h>
#include <openvdb/math/FiniteDifference.h>
#include <openvdb/util/NullInterrupter.h>
#include <vector>
#include <map>

int main(int argc, const char * argv[]) {
    openvdb::initialize();
    
    return 0;
}
