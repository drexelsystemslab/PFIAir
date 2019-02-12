#!/usr/bin/env python
from __future__ import print_function

import pandas

FEATURE_FILE = "input/shapenet/ShapeNetGroundTruth.csv"

def get_data():
    df = pandas.read_csv(FEATURE_FILE)
    # The UUID is only needed for locating the binvox files. For this
    # script we only care about the other columns.
    return df.drop(columns='UUID')

def cartesian_product(df1, df2):
    df1['_key'] = 0
    df2['_key'] = 0

    return (pandas.merge(df1, df2, on='_key', suffixes=('_source', '_target'))
        .drop(columns='_key'))

def gower_qualitative(row, feature):
    # Get the categorical values
    source = row[feature + '_source']
    target = row[feature + '_target']
    
    # Score 1 if they match, 0 if they don't
    if source == target:
        # (score, delta)
        return (1.0, 1.0)
    else:
        return (0.0, 1.0)

def gower_dichotomous(row, feature):
    # Get the boolean values
    source = row[feature + '_source']
    target = row[feature + '_target']

    # Score 1 if they are both True, 0 if one is true, and don't count 
    # a score it if neither 
    if source and target:
        return (1.0, 1.0)
    elif source and not target or target and not source:
        return (0.0, 1.0)
    else:
        return (0.0, 0.0)

def gower_quantitative(row, feature, val_range):
    # Get the boolean values
    source = row[feature + '_source']
    target = row[feature + '_target']

    diff = 1.0 - abs(source - target) / val_range
    return (diff, 1.0) 

def compute_dissimilarity(row):
    # Apply the gower scoring functions for each feature
    results = [
        gower_qualitative(row, 'Category'),
        gower_qualitative(row, 'Long/Flat/Solid'),
        gower_quantitative(row, 'Surface Complexity', 10.0),
        gower_quantitative(row, 'Density', 10.0),
        gower_dichotomous(row, 'Has Thin Details'),
        gower_quantitative(row, 'N-fold rotational symmetry', 11.0),
        gower_quantitative(row, 'Reflection Axes', 11.0),
    ]

    score = 0.0
    num_valid = 0.0
    for (s, delta) in results:
        score += s
        num_valid += delta

    similarity = score / num_valid

    # dissimilarity = 1 - similarity
    return 1.0 - similarity

def main():
    # Read in the CSV file
    data_frame = get_data() 

    # Make the full N x N table of model info
    n_by_n = cartesian_product(data_frame, data_frame)

    # Apply a dissimilarity function to each row
    n_by_n['dissimilarity'] = n_by_n.apply(compute_dissimilarity, axis=1)

    # Reduce the table to just the model names and dissimilarity
    n_by_n = n_by_n[
        ['Model Name_source', 'Model Name_target', 'dissimilarity']]

    n_by_n.to_csv('output/expected_dissimilarity.csv')
    

if __name__ == "__main__":
    main()
