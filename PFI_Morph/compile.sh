#!/bin/bash
if [[ "$1" == "release" ]]
then
    FLAGS='-DCMAKE_BUILD_TYPE=Release'
else
    FLAGS='-DCMAKE_BUILD_TYPE=Debug'
fi

# Do an out-of-source build using CMake
cd build
cmake $FLAGS ..
make -j 4

# Put the executable in the current directory
cp PFI_Morph ..

