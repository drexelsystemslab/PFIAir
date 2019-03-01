# Shape Dissimilarity Using Level Set Morphing

## Introduction

TODO

## Dependencies

### OpenVDB

The key C++ library required for this project is OpenVDB. It has its own
dependencies. Here is a minimum that needs to be installed, as well as
links to dependencies:

* OpenVDB https://www.openvdb.org/download/
* (libtbb) Threading Building Blocks https://www.threadingbuildingblocks.org/
* OpenEXR, specifically libHalf for half-precision floats.
    http://www.openexr.com/
* Boost (https://www.boost.org/)

See OpenVDB's INSTALL file for more detailed instructions.

NOTE: This project assumes OpenVDB is installed in the default system
location so the libraries can be found in the default system include paths.

### Python dependencies

The python dependencies can be installed via pip as is typical:

`pip install -r requirements.txt`

It is recommended to do this in a virtualenv, but that is up to the user's
discretion.

## Installation and Configuration

This project requires a few steps of installation and configuring where
to put output files

### Compile Cython code

Run the helper script `./compile_cython.sh` to compile the C++ and Cython
code into two Python extension modules, `pfimorph/binvox2vdb.so` and
`pfimorph/lsmorph.so`

NOTE: this step can take quite a while. OpenVDB is a header-only library
so it takes a while to compile

### Input .binvox files

TODO

## Usage

TODO

### Level Set Morphing: `morph_models.py`

TODO

### Post Process Morph Results: `post_process.py`

TODO

### Generate Ground Truth Data: `generate_ground_truth.py`

TODO

### Utility Script: `compile_cython.sh`

This script includes the current command to compile the Cython/C++ code into
the required location.

Usage:

`./compile_cython.sh`

### Utility Script: `find_shapenet_models.sh`

TODO

### Optional Utility Script: `binvox_to_vdb.py`

TODO

## Project Structure

This is an outline of files in this repository, along with a description
of what they do.

```
PFI_Morph/
|--build/               Cython build files will be generated here.
|
|--cpp/
|  |--binvox2vdb/       C++ source for binvox -> VDB file conversion
|  |  |--*.h
|  |  |--*.cpp
|  |
|  |--lsmorph/          C++ source for the level set morphing code.
|     |--*.h
|     |--*.cpp
|
|--pfimorph/            Python/Cython source for orchestrating morphing
|  |--templates/        Jinja2 templates for generating reports
|  |--*.py              Python helper source code
|  |
|  |--binvox2vdb.pxd    Cython wrapper around the binvox2vdb C++ module
|  |--binvox2vdb.pyx
|  |
|  |--lsmorph.pxd       Cython wrapper around the lsmorph C++ module
|  |--lsmorph.pyx
|
|--input/               Input .binvox models will go here
|  |
|  |--model_index.csv           List of models from the ShapeNet database with
|  |                            their UUIDs, along with a list of human-defined
|  |                            features to describe each model for creating
|  |                            the ground truth data. NOTE: This file can
|  |                            be modified by the user to select different
|  |                            models
|  |--find_shapenet_models.sh   Utility script to locate ShapeNet models via
|  |                            UUIDs usiing ShapeNetModels.csv. The models
|  |                            are always put in the binvox folder in this
|  |                            directory, it is up to the user to move
|  |                            them if a different directory is desired
|  |--binvox/
|     |--*.binvox               (Not Included) binvox files either fetched
|                               from a copy of the ShapeNet dataset or supplied
|                               by the user.
|--output/                  Default output directory for this project. This
|  |                        can be reconfigured in `pfimorph.cfg`
|  |--preprocessed_vdbs/    After converting .binvox -> .vdb files, the results
|  |  |--*.vdb
|  |                        are cached here.
|  |--morph_animations/     If the user opts to save .vdb files for each frame
|  |  |--*.vdb              in the morph animation, they will show up here.
|  |
|  |--morph_cache/          Cache of .json files that store the calculated 
|  |  |--*.json             stats from each pair of morphs. If the initial 
|  |                        processing has to be stopped halfway through, this 
|  |                        allows the user to continue from the last completed
|  |                        morph.
|  |--reports/          HTML files of the morphing results and other reports
|  |  |--morph_table.html       HTML table representation of 
|  |  |                         output/morph_table.csv for easier viewing
|  |  |--<model>_all.html       After morphing, one report is generated
|  |                           per model, including graphs of the morph data
|  |--analytics/            Graphs and other miscellaneous data files
|                           from the analysis script will go here.
|
|--setup.py             Build script for Cython modules (binvox2vdb, lsmorph)
|--pfimorph.cfg         Config file so the user can specify where input/output
                        files will go. The example file given includes
                        comments about what each directory is for
```
