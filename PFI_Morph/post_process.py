#!/usr/bin/env python
from __future__ import print_function
import json
import os

import pandas

def get_fnames():
    """
    Crawl through the output/json/ directory and
    gather up all json files

    This assumes that there are no extraneous files
    """
    for root, _, files in os.walk('output/json'):
        for f in files:
            yield os.path.join(root, f)

def get_stat_pairs():
    """
    Iterate over .json files in the output/json/ directory
    and generate a list of dictionaries of the contents
    """
    for fname in get_fnames():
        with open(fname, 'r') as f:
            yield json.load(f)

def get_forward_stats(pair):
    """
    Pull out stats from the forwards morph
    """
    forward = {
        'source': pair['source_name'],
        'target': pair['target_name'],
        'curves': pair['forward_curves'],
    }
    zipped = zip(pair['stat_labels'][1:-1], pair['forward_stats'])
    for label, val in zipped:
        forward[label] = val

    return forward 

def get_backward_stats(pair):
    """
    Pull out stats from the backwards morph
    """
    backward = {
        'source': pair['target_name'],
        'target': pair['source_name'],
        'curves': pair['backward_curves'],
    }

    zipped = zip(pair['stat_labels'][1:-1], pair['backward_stats'])
    for label, val in zipped:
        backward[label] = val

    return backward
    

def split_stat_pairs(pairs):
    """
    Split the stat pairs into a sequence of pairs (forward, backward)

    Since the morphs can be presented in either direction, normalize them
    so the "forward" morph always goes from smaller model -> larger model
    """
    for pair in pairs:
        # skip cases where source = target, they're not going to be 
        # helpful for this analysis
        if pair['source_name'] == pair['target_name']: 
            continue

        forward = get_forward_stats(pair)
        backward = get_backward_stats(pair)
  
        # Normalize the pairs such that "forward" is
        # from the smaller model (in terms of voxels) to the larger
        # model
        voxel_count = lambda stats: stats['Source Voxel Count']
        smaller_source, larger_source = sorted(
            [forward, backward], key=voxel_count)
        yield smaller_source, larger_source

COLUMN_ORDER = [
    'source',
    'target',
    'source_voxels',
    'target_voxels',
    'abs_diff_voxels',
    'avg_voxels',
    'fwd_time_steps',
    'bwd_time_steps',
    'fwd_cfl',
    'bwd_cfl',
    'fwd_evolving_voxel_avg',
    'bwd_evolving_voxel_avg',
    'fwd_max_curv',
    'bwd_max_curv',
    'fwd_total_curv',
    'bwd_total_curv',
    'fwd_curv_over_area',
    'bwd_curv_over_area',
    'fwd_total_val',
    'bwd_total_val',
    'fwd_val_over_area',
    'bwd_val_over_area',
]

def make_rows(morph_data):
    """
    Combine the forward -> backward morphs into one big row.

    This is a good place to add additional stats
    """
    for forward, backward in morph_data:
        source_voxels = forward['Source Voxel Count']
        target_voxels = forward['Target Voxel Count']
        diff_voxels = abs(source_voxels - target_voxels)
        avg_voxels = (source_voxels + target_voxels) / 2.0

        # Recompute curvature and value divided by voxels
        # without any arbitrary scaling factors
        fwd_evolving_avg = forward['Evolving Average']
        fwd_total_curv = forward['Total Curvature']
        fwd_curv_over_area = fwd_total_curv / fwd_evolving_avg
        fwd_total_val = forward['Total Value']
        fwd_val_over_area = fwd_total_val / fwd_evolving_avg

        bwd_evolving_avg = backward['Evolving Average']
        bwd_total_curv = backward['Total Curvature']
        bwd_curv_over_area = bwd_total_curv / bwd_evolving_avg
        bwd_total_val = backward['Total Value']
        bwd_val_over_area = bwd_total_val / bwd_evolving_avg

        yield {
            'source' : forward['source'],
            'target' : forward['target'],
            'source_voxels': source_voxels,
            'target_voxels': target_voxels,
            'abs_diff_voxels': diff_voxels,
            'avg_voxels': avg_voxels,
            'fwd_time_steps': forward['Time steps'],
            'bwd_time_steps': backward['Time steps'],
            'fwd_cfl': forward['CFL Iterations'],
            'bwd_cfl': backward['CFL Iterations'],
            'fwd_evolving_voxel_avg': forward['Evolving Average'],
            'bwd_evolving_voxel_avg': backward['Evolving Average'],
            'fwd_max_curv': forward['Max Curvature'],
            'bwd_max_curv': backward['Max Curvature'],
            'fwd_total_curv': forward['Total Curvature'],
            'bwd_total_curv': backward['Total Curvature'],
            'fwd_curv_over_area': fwd_curv_over_area,
            'bwd_curv_over_area': bwd_curv_over_area,
            'fwd_total_val': fwd_total_val,
            'bwd_total_val': bwd_total_val,
            'fwd_val_over_area': fwd_val_over_area,
            'bwd_val_over_area': bwd_val_over_area,
        }

def make_data_frame():
    """
    Turn everything into a Pandas DataFrame
    """
    morphs = split_stat_pairs(get_stat_pairs())
    rows = make_rows(morphs)
    df = pandas.DataFrame.from_records(rows, columns=COLUMN_ORDER)
    return df.set_index(['source', 'target']).sort_index()

def main():
    table = make_data_frame()
    
    # Generate an HTML table for viewing in the browser
    with open('Reports/test_table.html', 'w') as f:
        f.write('<html><body>{}</body></html>'.format(table.to_html()));

    # and a CSV file for use in a later script for machine learning
    table.to_csv('output/test_table.csv')

if __name__ == "__main__":
    main()
