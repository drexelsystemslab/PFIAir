#!/usr/bin/env python
from __future__ import print_function
import os
import argparse

import trimesh

import pfimorph

CONVERTED_DIR = 'converted_objs'

def make_directories(dirname):
    """
    Make a directory if it doesn't already exist
    """
    if not os.path.exists(dirname):
        os.makedirs(dirname)

def morph_all_pairs(args):
    print("TODO: Morph all pairs")

def morph_single_pair(args):
    """
    morph args.source_model into args.target_model
    """
    print(args)
    print("Morphing {} <-> {}".format(args.source_model, args.target_model))

    # TODO: return a MorphStats object
    # Quick test that we can do this
    energy = pfimorph.morph(args.source_model, args.target_model)
    print(energy)

def stl_to_obj(stl_fname):
    """
    Given an STL filename, convert it to an OBJ file if we haven't
    already. Then return the converted OBJ filename
    """
    # Get the model name minus the path and extension
    _, fname = os.path.split(stl_fname)
    model_name, _ = os.path.splitext(fname)

    # new output file is in the converted model directory
    new_fname = os.path.join(CONVERTED_DIR, model_name + '.obj')

    # Convert the model if it doesn't already exist
    if not os.path.exists(new_fname):
        make_directories(CONVERTED_DIR)
        mesh = trimesh.io.load.load(stl_fname)
        trimesh.io.export.export_mesh(mesh, new_fname)

    return new_fname

def mesh_fname(fname):
    """
    Argparse type function for picking a mesh filename. 
   
    If the mesh is in .obj format, the filename is returned unmodified

    If the mesh is in .stl format, convert the model to {CONVERTED_DIR}/foo.obj
    and return this new path.
    """
    if fname.endswith('.obj'):
        # OBJ files are the desired format
        return fname
    elif fname.endswith('.stl'):
        # Convert the model into an object file
        return stl_to_obj(fname)
    else:
        # No other model formats are expected
        raise argparse.ArgumentTypeError(
            "'{}': Filename must end in .obj or .stl".format(fname))

def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()
    subparsers.required = True
    
    # Morph all pairs of models
    morph_all = subparsers.add_parser('all')
    morph_all.set_defaults(func=morph_all_pairs)

    # Morph a single pair of models and generate a report
    morph_one = subparsers.add_parser('one')
    morph_one.add_argument('source_model', type=mesh_fname,
        help="Path to source model in OBJ/STL format")
    morph_one.add_argument('target_model', type=mesh_fname,
        help="Path to target model in OBJ/STL format")
    morph_one.set_defaults(func=morph_single_pair)
    
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    args.func(args)
