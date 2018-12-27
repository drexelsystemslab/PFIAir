#!/usr/bin/env python
from __future__ import print_function
import os
import argparse
import math
import glob
import multiprocessing
import functools

from jinja2 import Environment, FileSystemLoader, select_autoescape
import trimesh

import pfimorph

CONVERTED_DIR = 'converted_objs'
MODELS = glob.glob('open_mesh_objs/all_pairs/*')

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

def morph_pair_parallel(args, model_names):
    """ 
    Morph a single pair of models. This is designed to be used
    in multiprocessing.pool
    """
    # filenames are passed in
    source_fname, target_fname = model_names
    source_name = get_short_name(source_fname)
    target_name = get_short_name(target_fname)
    source_open = is_open_mesh(source_fname)
    target_open = is_open_mesh(target_fname)

    morpher = pfimorph.Morpher()
    morpher.set_source_info(source_fname, source_name, source_open)
    morpher.set_target_info(target_fname, target_name, target_open)
    stat_pair = morpher.morph(
        cache=args.cache_enabled, 
        save_debug_models=args.save_debug_models, 
        profile=args.profile,
        max_iters=args.iter_limit) 
    return stat_pair

def morph_all_pairs(args):
    """
    Morph all pairs of models from the given list
    """
    N = len(MODELS)

    # Since morphing happens bi-directionally, we only need to iterate
    # over the upper triangular portion.
    # Let's pre-compute the indexes and model names. Then we can
    # use multiprocessing to compute everything in parallel
    indices = []
    model_pairs = [] 
    for i in xrange(N):
        for j in xrange(i, N):
            indices.append((i, j))
            model_pairs.append((MODELS[i], MODELS[j]))
    
    print(indices)
    print(model_pairs)

    print("Start morphing pairs. This could take a while...")
    func = functools.partial(morph_pair_parallel, args)

    # TODO: Figure out how to pickle Cython objects
    # so I can use multiprocessing.pool
    #cpus = multiprocessing.cpu_count()
    #pool = multiprocessing.Pool(cpus)
    #stat_pairs = pool.map(func, model_pairs)
    stat_pairs = [func(pair) for pair in model_pairs]

    # Make an NxN table of morph results. For the lower triangular portion,
    # just swap source and target, since all morphs are done bidirectionally
    table = [[None] * N for i in xrange(N)]
    for (i, j), stat_pair in zip(indices, stat_pairs):
        if i == j:
            table[i][j] = stat_pair
        else:
            table[i][j] = stat_pair
            table[j][i] = stat_pair.swapped

    # Make the reports by iterating over the rows
    for i in xrange(N):
        source = get_short_name(MODELS[i])
        report_fname = "Reports/{}_all.html".format(source)
        write_report(report_fname, table[i])

def morph_single_pair(args):
    """
    morph args.source_model into args.target_model
    """
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
    stat_pair = morpher.morph(
        cache=args.cache_enabled, 
        save_debug_models=args.save_debug_models, 
        profile=args.profile,
        max_iters=args.iter_limit)

    # Save a report
    report_fname = "Reports/{}_{}.html".format(source_name, target_name)
    write_report(report_fname, [stat_pair])

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
    stat_pair = morpher.morph(
        cache=args.cache_enabled, 
        save_debug_models=args.save_debug_models, 
        profile=args.profile)

    # Save a report
    report_fname = "Reports/{}_{}.html".format(source_name, target_name)
    write_report(report_fname, [stat_pair])

def format_number(num):
    # Can't compute log10(0)
    if (int(num) == 0):
        return "0"

    # Determine which group of three digits we are in.
    # group 0: 0 to 999
    # group 1: 1,000 to 999,999
    # group 2: 1,000,000 to 999,999,999
    num_digits = int(math.floor(math.log10(abs(num))))
    nearest_thousands_group = num_digits // 3

    # Round to the nearest thousands group so it's easier to read
    round_position = -3 * nearest_thousands_group
    rounded = round(num, round_position)

    # Add in thousands separators and trim decimals
    return "{:,.0f}".format(rounded)
    

def write_report(fname, stat_pairs):
    """
    Generate a HTML report using Jinja2
    """
    # Set up a Jinja templating environment, including a custom
    # format function for numbers
    env = Environment(
        loader=FileSystemLoader("templates"),
        autoescape=select_autoescape(['html', 'xml']))
    env.filters['format_number'] = format_number

    template = env.get_template('report.html')
    html = template.render(
        source_name=stat_pairs[0].source_name,
        stat_pairs=stat_pairs,
        columns=pfimorph.StatPair.COLUMNS)

    with open(fname, 'w') as f:
        f.write(html) 

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
    parser.add_argument('-p', '--profile', action='store_true',
        help="Set this flag to time each step of the program")
    parser.add_argument('-s', '--save-debug-models', action='store_true',
        help="Set this flag to save extra .obj and .vdb models")
    parser.add_argument(
        '-d', 
        '--disable-cache', 
        dest='cache_enabled', 
        action='store_false',
        help="Set this flag to disable using the cache")
    parser.add_argument(
        '-i', '--iter-limit', type=int, default=20, 
        help="Set max number of frames for each morph (default 20)")

    subparsers = parser.add_subparsers()
    subparsers.required = True
    
    # Morph all pairs of models
    morph_all = subparsers.add_parser('all')
    morph_all.set_defaults(func=morph_all_pairs)
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
