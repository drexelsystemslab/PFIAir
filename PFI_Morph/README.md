# Shape Dissimilarity Using Level Set Morphing

## Introduction

This project attempts to find a new shape dissimilarity metric by using
level set morphing.

This project has only been tested on Linux in a Python 2.7 environment.

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

### Input .binvox files (ShapeNet Core)

The input to this script is the ShapeNet Core database, specifically the
solid .binvox models they provide. To install and configure these models for 
the project, do the following:

1. Visit (https://shapenet.org/) and follow the instructions there for
    requesting an account and downloading the models
2. Download and extract the ShapeNet Core dataset. NOTE: This is a rather large
    data set, ~100GB after uncompressing.
3. Make a note of the installation directory
4. Review `input/model_index.csv` and modify if desired (see below)
5. navigate into the `input/` directory and run the command
    `./find_shapenet_models.sh <path/to/shapenet/here>`

### Model Index Configuration File

The file `input/model_index.csv` is a configuration file for selecting which
ShapeNet models to use for morphing. The format is a CSV file with the given
format:

* Model Name - Pick a unique human-readable ID for each model. This will be
    used for filenames and reports.
* UUID - The UUID of the model in the ShapeNet database. This is used to locate
    the correct folder in the database.
* Category - A human-defined category for the models. This was intended to
    be used as an expected value for clustering, but right now is only used
    for organizing the heatmap by category.
* Human-Defined Features - The remaining columns are features defined by a
    human to distinguish the models and establish an "expected dissimilarity"
    by Gower dissimilarity. 

25 models have been selected out of the data set with features defined.
The user is free to add/remove rows as desired. NOTE: The run time of the
morphing depends on the *square* of the number of models, so be wary of this
when selecting models

Adding/removing features is more involved. If you do this, make sure to
update `./ground_truth.py` to interpret the columns properly.

### Directory Configuration

All the input directories and output directories of this program are listed
in `pfimorph.cfg`. Read the comments there to learn what each one does.

The directories default to the subfolders of this project. This is normally
a recommended location for most files. **IMPORTANT**, there is one exception.
the `morph_animations_dir` should be configured to point somewhere where you
can store several GB worth of output data. This directory is only populated
if the `-s/--save-debug-models` flag of `./morph_models.py` is specified, but
when it is populated, you may wind up with a few thousand .vdb files.

## Usage Guide

This project includes several Python scripts. This is an overview of what
each one does.

### Level Set Morphing: `morph_models.py`

This script is the main script for level set morphing of the models and
computing statistics.

There are two ways to run this script. The first is for morphing a single
pair of models, mainly to test that the morph code is working

```
./morph_models.py [OPTIONS] one <source>.binvox <target>.binvox
```

This runs the morphing for just one pair of models. It does generate a
HTML report, but it does not generate the morph cache JSON data.

The other way to run this script is to run it on all pairs of models. This
can be done with the following command:

```
./morph_models.py [OPTIONS] all
```

Either way, the options are as follows:

* `-p/--profile` - Enable this flag to time each step of the program
* `-s/--save-debug-models` - If you want to save the morph animations as
    .vdb files. IMPORTANT: This can generate several GB of data depending on
    how many input models there are. Make sure to configure the output
    directories before enabling this flag!
* `-i/--iter-limit N` - This is a safeguard against the rare case where morphs
    do not converge. This issue has not been seen since the last major change,
    but nevertheless it doesn't hurt. IMPORTANT: This defaults to 20 iterations
    to prevent accidental runs of the script during development. Set it to 
    something higher (500 recommended) when doing a real run. `run_all.sh`
    does this automatically.
* `-m/--model-limit N` If you wish to do a quick run for testing the script,
    you can use this flag to run only the first N models instead of all of
    them.

This script does the following:

1. Select which models to use. For the `one` command, it takes the two
    binvox files specified by the user. for the `all` command, it takes the
    list of models and creates a list of model pairs. `(model a, model b)` is
    considered the same as `(model b, model a)`
2. The models are looked up in the VDB preprocessed model cache. If a model
    is not found, the `binvox2vdb` Cython module is used to convert the .binvox
    file to a .vdb file.
3. (`all` only) For each morph pair that needs to be performed, check if there
    is a JSON cache file. If so, skip this morph, it's already been done.
4. Use the `lsmorph` Cython module to morph the pair of models both forwards
    and backwards. The stats about curvature and voxels over time are returned
5. (`all` only) Save the results as a JSON cache file so A. if a partial
    re-run is required, less work needs to be done. B. These files are used
    by the post-processing and analysis scripts.
6. Generate a HTML report of the morph stats. These include graphs of the
    morph data over the duration of the morph to help inspect the results.

### Post Process Morph Results: `post_process.py`

After running the morphing code in the NxN configuration, the next step
is to reorganize the output into a format that is easier to analyze. This
script handles such steps.

Usage:

```
./post_process.py
```

This script does the following:

1. Read in the JSON files from the morph cache produced by the level set
    morphing script.
2. Flatten the JSON data into rows of floating point features. The time
    series information is not included, since that is only used in the
    report. Also, some stats are re-computed to avoid old scaling factors
    that are no longer used.
3. Output the table as a CSV file (the morph table) and a HTML table for
    easier viewing inn a browser.

### Generate Ground Truth Data: `ground_truth.py`

This script creates a "expected values" data set for the models so there
is something to train against.

Usage:

```
./ground_truth.py
```

The model index file (see section on this above) contains not only the
model UUID, but several human-defined features to describe the model.
The ground truth script turns these features into an expected dissimilarity
metric. This is done as follows:

1. Read in thhe CSV file for the model index.
2. Compute the cartesian product of the table with itself so all nxn models
    are listed
3. Compute Gower Similarity on each row, and subtract from 1 to get
    an expected dissimilarity.
4. Now we have a table of `(source model, target model) -> dissimilarity`.
    Write this to a CSV file
5. Also generate a heatmap plot of the dissimilarity for visual feedback of
    the quality of the expected dissimilarity.

See the paper "A General Coefficient of Similarity and Some of Its Properties"
by J. C. Gower from *Biometrics*, Vol. 27, No. 4. (Dec., 1971), pp. 857-871)
for more information about the Gower Similarity metric.

### Analyze Results: `analyze_morphs.py`

Once the morphing, post-processing, and ground truth steps have been completed,
we can analyze the results using this script.

Usage:

```
./analyze_morphs.py
```

This script does the following:

1. Read in the post-processed data as well as the ground truth data
2. Combine them into a big table with 
3. Use Scikit-Learn to apply linear regression to the data
4. Plot the results, both as a graph and a heatmap
5. Display the model coefficients listed by magnitude of the coefficients
    descending. This gives a rough idea of what features are most important.

**NOTE:** Due to some setbacks and confusion over the exact deadline, this
script is not as fleshed out as the morphing code. Only a simple linear
regression is applied. It is highly recommended to apply other models
to the data set to find better fits for the data.

### Utility Script: `compile_cython.sh`

This script includes the current command to compile the Cython/C++ code into
the required location.

Usage:

`./compile_cython.sh`

### Utility Script: `input/find_shapenet_models.sh`

This shell script is used for locating the ShapeNet models by UUID and placing
them in the `./input/binvox/` directory.

Usage:

```
./find_shapenet_models.sh <shapenet_directory> [path/to/model_index.csv]
```

This script pulls out the UUIDs from the model index file and searches
through the ShapeNet database for the models. Finally, the models are copied
to the input directory.

### Optional Utility Script: `binvox_to_vdb.py`

This script converts a .binvox file to a .vdb file. It is not necessary for
this project, but it may come in handy someday.

Usage:

```
./binvox_to_vdb.py <in_file>.binvox <out_file>.vdb
```

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
|  |                        morph. These files are also used as input to the
|  |                        analysis scripts
|  |                        
|  |--reports/          HTML files of the morphing results and other reports
|  |  |--morph_table.html       HTML table representation of 
|  |  |                         output/morph_table.csv for easier viewing
|  |  |--<model>_all.html       After morphing, one report is generated
|  |  |                         per model, including graphs of the morph data
|  |  |
|  |  |--styles.css             Stylesheet for the report
|  |  |--scripts/graph.js       Needed to display the graphs in each report
|  |  
|  |--analytics/            Graphs and other miscellaneous data files
|                           from the analysis script will go here.
|
|--requirements.txt     File that lists Python dependencies.
|--setup.py             Build script for Cython modules (binvox2vdb, lsmorph)
|--pfimorph.cfg         Config file so the user can specify where input/output
|                       files will go. The example file given includes
|                       comments about what each directory is for
|--compille_cython.sh   This shell script is for compiling the Cython and C++
|                       code.
|--morph_models.py      This is the main script for morphing the binvox models
|                       to generate statistics about dissimilarity
|--post_process.py      This script is run after `morph_models.py`. It reads
|                       in the JSON files in the morph cache and rearranges
|                       the data into a big table of floating point features.
|--ground_truth.py      Process the ground truth features to get a table of
|                       "expected" dissimilarity to compare with.
|--analyze_morphs.py    Run machine learning on the output of post_process.py
|                       and ground_truth.py 
|--binvox_to_vdb.py     (Optional) This is a little utility script for
|                       converting a .binvox file to a .vdb file. It is not
|                       needed for the main scripts.
|--run_all.sh           This shell script contains the commands for running
                        the scripts in a valid sequence.
```
