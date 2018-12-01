#!/usr/bin/env python
from __future__ import print_function
import argparse
#import pfimorph

def morph_all_pairs(args):
    print("TODO: Morph all pairs")

def morph_single_pair(args):
    print("TODO: Morph single pair")

def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()
    subparsers.required = True
    
    # Morph all pairs of models
    morph_all = subparsers.add_parser('all')
    morph_all.set_defaults(func=morph_all_pairs)

    # Morph a single pair of models and generate a report
    morph_one = subparsers.add_parser('one')
    morph_one.add_argument('source_model', 
        help="Path to source model in OBJ/STL format")
    morph_one.add_argument('target_model',
        help="Path to target model in OBJ/STL format")
    morph_one.set_defaults(func=morph_single_pair)
    
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    args.func(args)
