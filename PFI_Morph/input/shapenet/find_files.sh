#!/bin/bash

# Get the first 2 columns from the CSV file, ignoring the header row
function get_uuids {
    # column 1 is my human-readable model IDs. column 2 is the official
    # ShapeNet UUID
    awk -F ',' 'NR > 1 {print $1, $2}' ShapeNetGroundTruth.csv
}

# This is where I keep the ShapeNet database
SHAPENET_DIR=~/RES/data_partition/ShapeNetCore.v2

# Unfortunately, all the models have the same generic filename
MODEL_FNAME=models/model_normalized.solid.binvox

# Iterate over the models specified in the CSV file
while read model_name uuid
do
    # Glob for the model file by using the UUID.
    # Albeit a brute-force solution, it's a lot simpler than
    # trying to match UUIDs to their Synset ID
    for model in $SHAPENET_DIR/*/$uuid/$MODEL_FNAME
    do
        # Copy the model to this directory, giving it a better name
        cp $model ./$model_name.binvox

    done
done < <(get_uuids)
