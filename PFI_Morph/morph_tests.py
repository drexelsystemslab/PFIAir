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

def is_open_mesh(obj_file):
    """
    Check if a model is closed or open
    """
    mesh = trimesh.io.load.load(obj_file);
    return not mesh.is_watertight

def morph_all_pairs(args):
    print("TODO: Morph all pairs")

def get_short_name(path):
    """
    Strip the path and extension from a filename to get a shorter name
    """
    _, fname = os.path.split(path)
    short_name, _ = os.path.splitext(fname)
    return short_name

def morph_single_pair(args):
    """
    morph args.source_model into args.target_model
    """
    print(args)
    print("Morphing {} <-> {}".format(args.source_model, args.target_model))

    # Check if we have open/closed meshes
    source_open = is_open_mesh(args.source_model)
    target_open = is_open_mesh(args.target_model)
    print("Source model is open mesh: {}".format(source_open))
    print("Target model is open mesh: {}".format(target_open))

    # Get a shorter name for each model
    source_name = get_short_name(args.source_model)
    target_name = get_short_name(args.target_model)

    # Run the morphinng
    morpher = pfimorph.Morpher()
    morpher.set_source_info(args.source_model, source_name, source_open)
    morpher.set_target_info(args.target_model, target_name, target_open)
    energy = morpher.morph(
        cache=args.cache_enabled, 
        save_debug_models=args.save_debug_models, 
        profile=args.profile)
    print(energy)

def stl_to_obj(stl_fname):
    """
    Given an STL filename, convert it to an OBJ file if we haven't
    already. Then return the converted OBJ filename
    """
    # Get the model name minus the path and extension
    model_name = get_short_name(stl_fname)

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
    morph_one.add_argument('-p', '--profile', action='store_true',
        help="Set this flag to time each step of the program")
    morph_one.add_argument('-s', '--save-debug-models', action='store_true',
        help="Set this flag to save extra .obj and .vdb models")
    morph_one.add_argument(
        '-d', 
        '--disable-cache', 
        dest='cache_enabled', 
        action='store_false',
        help="Set this flag to disable using the cache")
    morph_one.set_defaults(func=morph_single_pair)
    
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    args.func(args)
