#!/usr/bin/env python
from __future__ import print_function
import os

import pandas

from pfimorph.config import config
from pfimorph import plots

def get_data():
    model_index = config.get('input', 'model_index')
    df = pandas.read_csv(model_index)
    # The UUID is only needed for locating the binvox files. For this
    # script we only care about the other columns.
    return df.drop(columns='UUID').sort_values('Category')

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

    # Output directory
    analytics_dir = config.get('output', 'analytics_dir')

    # for the detailed view, drop all features but the dissimilarity
    adjacency_list = n_by_n.set_index(
        ['Model Name_source', 'Model Name_target'])['dissimilarity']

    # Output a CSV file of the data and a heatmap for an easier view
    dissim_csv = os.path.join(analytics_dir, 'expected_dissimilarity.csv')
    adjacency_list.to_csv(dissim_csv)
    print("Created Dissimilarity Data File, {}".format(dissim_csv))
    dissim_png = os.path.join(analytics_dir, 'expected_dissimilarity.png')
    plots.make_heatmap('Expected Dissimilarity', adjacency_list, dissim_png)
    print("Created Dissimilarity Heatmap, {}".format(dissim_png))

    # For a higher-level view, let's just average the categories
    category_avgs = n_by_n.groupby(
        ['Category_source', 'Category_target']).mean()['dissimilarity']

    # Again, output a CSV file and a heatmap
    category_csv = os.path.join(analytics_dir, 'expected_category_avgs.csv')
    category_avgs.to_csv(category_csv)
    print("Created Category Avg. Data File, {}".format(category_csv))
    category_png = os.path.join(analytics_dir, 'expected_category_avgs.png')
    plots.make_heatmap('Expected Dissimilarity', category_avgs, category_png)
    print("Created Category Avg. Heatmap, {}".format(category_png))

if __name__ == "__main__":
    main()
