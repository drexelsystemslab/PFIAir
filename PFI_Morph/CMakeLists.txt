cmake_minimum_required(VERSION 3.9)
project(PFI_Morph)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)


#find_package(libtbb)
#target_link_libraries (PFI_Morph /Users/Hollis/Developer/PFIAir/PFIAir/PFI_Morph/libtbb.dylib)

add_executable(PFI_Morph
        MorphingUsingLevelSets/Algorithm.cpp
        MorphingUsingLevelSets/Algorithm.hpp
        MorphingUsingLevelSets/CommonOperations.h
        MorphingUsingLevelSets/Container.cpp
        MorphingUsingLevelSets/Container.hpp
        MorphingUsingLevelSets/GridOperations.h
        MorphingUsingLevelSets/main.cpp
        MorphingUsingLevelSets/MeshOperations.h
        MorphingUsingLevelSets/MorphOperations.h
        MorphingUsingLevelSets/Tests.h)

target_link_libraries(PFI_Morph libtbb.dylib libopenvdb.4.0.2.dylib libHalf.12.dylib)

