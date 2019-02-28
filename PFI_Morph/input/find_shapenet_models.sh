#!/bin/bash
set -e

if [[ -z "$1" ]]
then
    echo "Usage: ./find_shapenet_models.sh <shapenet_directory> [model_index]"
    echo "Default Model Index: model_index.csv"
    exit 1
fi

# Path to the ShapeNet database
SHAPENET_DIR="$1"

# Path to the index CSV file. defaults to model_index.csv in the same
# directory
INDEX_FILE=${2:-model_index.csv}

# Get the first 2 columns from the CSV file, ignoring the header row
function get_uuids {
    # column 1 is my human-readable model IDs. column 2 is the official
    # ShapeNet UUID
    awk -F ',' 'NR > 1 {print $1, $2}' $INDEX_FILE
}


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
        cp $model ./binvox/$model_name.binvox

    done
done < <(get_uuids)
