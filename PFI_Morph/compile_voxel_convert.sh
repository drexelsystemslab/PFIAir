#!/bin/bash
if [[ "$1" == "release" ]]
then
    FLAGS='-DCMAKE_BUILD_TYPE=Release'
elif [[ "$1" == "debug" ]]
then
    FLAGS='-DCMAKE_BUILD_TYPE=Debug'
else
    # Default to release with debug symbols
    # so the program runs faster
    FLAGS='-DCMAKE_BUILD_TYPE=RelWithDebInfo'
fi

# Do an out-of-source build using CMake
cd build
cmake $FLAGS ..
make -j 4 PFI_VoxelConvert

# Put the executable in the current directory
cp PFI_VoxelConvert ..

