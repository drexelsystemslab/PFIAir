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

def split_stat_pairs(pairs):
    """
    Take a sequence of stat pairs and split them into two
    rows: forward info and backward info
    """
    for pair in pairs:
        forward = {
            'source': pair['source_name'],
            'target': pair['target_name'],
            #'curves': pair['forward_curves'],
        }
        zipped = zip(pair['stat_labels'][1:-1], pair['forward_stats'])
        for label, val in zipped:
            forward[label] = val
        yield forward

        # If source = target model, we can skip the rest
        # since that would be redundant
        if forward['source'] == forward['target']: 
            continue

        backward = {
            'source': pair['target_name'],
            'target': pair['source_name'],
            #'curves': pair['backward_curves'],
        }
        zipped = zip(pair['stat_labels'][1:-1], pair['backward_stats'])
        for label, val in zipped:
            backward[label] = val
        yield backward

def make_data_frame():
    """
    Turn everything into a Pandas DataFrame
    """
    rows = list(split_stat_pairs(get_stat_pairs()))
    df = pandas.DataFrame()
    for key in rows[0]:
        df[key] = [row[key] for row in rows]
    return df.set_index(['source', 'target']).sort_index()
        
    

def main():
    table = make_data_frame()
    print(table)
    
    with open('Reports/test_table.html', 'w') as f:
        f.write('<html><body>{}</body></html>'.format(table.to_html()));

if __name__ == "__main__":
    main()
