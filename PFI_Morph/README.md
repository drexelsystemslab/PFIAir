# Shape Dissimilarity Using Level Set Morphing

## Introduction

TODO

## Dependencies

TODO

## Installation and Configuration

This project requires a few steps of installation and configuring where
to put output files

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

TODO

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
|--cython/
|  |--binvox2vdb.pxd    Cython wrapper around the binvox2vdb C++ module
|  |--binvox2vdb.pyx
|  |
|  |--lsmorph.pxd       Cython wrapper around the lsmorph C++ module
|  |--lsmorph.pyx
|
|--pfimorph/            High-level Python source for orchestrating morphing
|  |--templates/        Jinja2 templates for generating reports
|  |--*.py              Python helper source code
|  |
|  |--binvox2vdb.so     (Generated) The compiled Cython extensions go here
|  |--lsmorph.so
|
|--input/               Input .binvox models will go here
|  |
|  |--ShapeNetModels.csv        List of models from the ShapeNet database with
|  |                            their UUIDs, along with a list of human-defined
|  |                            features to describe each model for creating
|  |                            the ground truth data. NOTE: This file can
|  |                            be modified by the user
|  |--find_shapenet_models.sh   Utility script to locate ShapeNet models via
|  |                            UUIDs usiing ShapeNetModels.csv
|  |--*.binvox                  (Not Included) binvox files either fetched
|                               from a copy of the ShapeNet dataset or supplied
|                               by the user.
|--output/              Default output directory for this project. This
|  |                    can be reconfigured in the config file.
|  |--vdb_cache/        After converting .binvox -> .vdb files, the results
|  |                    are cached here. This directory can be 
|  |--morphs/           If the user opts to save .vdb files for each frame
|  |                    in the morph animation, they will show up here.
|  |--morph_cache/      Cache of .json files that store the calculated stats
|  |                    from each pair of morphs. If the initial processing
|  |                    has to be stopped halfway through, this allows the
|  |                    user to continue from the last complted morph.
|  |--reports/          HTML files of the morphing results and other reports
|  |  |--morph_table.html       HTML table representation of 
|  |  |                         output/morph_table.csv for easier viewing
|  |  |--<model>_all.html       After morphing, one report is generated
|  |                           per model, including graphs of the morph data
|  |--analysis_results  Graphs and other miscellaneous data files
|                       from the analysis script will go here.
|
|--setup.py             Build script for Cython modules (binvox2vdb, lsmorph)
```
