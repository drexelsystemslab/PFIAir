#!/bin/bash
# Do an out-of-source build using CMake
cd build
cmake ..
make

# Put the executable in the current directory
cp PFI_Morph ..

