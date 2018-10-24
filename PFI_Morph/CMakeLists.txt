cmake_minimum_required(VERSION 3.9)
project(PFI_Morph)

# use --std=c++11
set(CMAKE_CXX_STANDARD 11)

# Grab all the source files from the source directory
# IMPORTANT: Whenever a new .cpp file is added, run cmake again
FILE(GLOB_RECURSE source_files MorphingUsingLevelSets/*.cpp)

# List of required libraries. This assumes they are installed in their
# default system locations
LIST(APPEND libs openvdb tbb Half boost_iostreams)

# Build PFI_Morph executable
add_executable(PFI_Morph ${source_files})

# Link against openvdb and other libraries
target_link_libraries(PFI_Morph ${libs})

