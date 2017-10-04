#include <openvdb/openvdb.h>
#include "Tests.h"


int main()
{
    openvdb::initialize();
    
//    openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile("sphere.vdb"));
//    GridOperations::writeToFile("read_sphere.vdb", source_grid);
    Tests::performMorphPermutation();
    return 0;
}




