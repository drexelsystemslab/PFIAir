#!/usr/bin/env python
from __future__ import print_function
import argparse
import glob
import multiprocessing
import functools

from pfimorph_wrapper import util, morph, reports

MODELS = glob.glob('open_mesh_objs/all_pairs/*')

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
    
    print("Start morphing pairs. This could take a while...")
    func = functools.partial(cache_morph, args)

    # TODO: Figure out how to pickle Cython objects
    # so I can use multiprocessing.pool
    cpus = multiprocessing.cpu_count()
    pool = multiprocessing.Pool(cpus)
    stat_pairs = pool.map(func, model_pairs)
    #stat_pairs = [func(pair) for pair in model_pairs]

    print("Morphing done! Generating reports...")

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
        save_debug_models=args.save_debug_models, 
        profile=args.profile)


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
        return util.stl_to_obj(fname)
    else:
        # No other model formats are expected
        raise argparse.ArgumentTypeError(
            "'{}': Filename must end in .obj or .stl".format(fname))

def morph_all(args):
    print(args)

def morph_one(args):
    """
    morph args.source_model into args.target_model
    """
    print("Morphing {} <-> {}".format(args.source_model, args.target_model))
    stat_pair = morph.morph_pair(args, (args.source_model, args.target_model))

    # Save a report
    source_name = util.get_short_name(args.source_model)
    target_name = util.get_short_name(args.target_model)
    report_fname = "Reports/{}_{}.html".format(source_name, target_name)
    reports.write_report(report_fname, [stat_pair]) 

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('model_type', choices=['open', 'closed'])
    parser.add_argument('-p', '--profile', action='store_true',
        help="Set this flag to time each step of the program")
    parser.add_argument('-s', '--save-debug-models', action='store_true',
        help="Set this flag to save extra .obj and .vdb models")
    parser.add_argument(
        '-i', '--iter-limit', type=int, default=20, 
        help="Set max number of frames for each morph (default 20)")

    subparsers = parser.add_subparsers()
    subparsers.required = True
    
    # Morph all pairs of models
    parser_all = subparsers.add_parser('all')
    parser_all.set_defaults(func=morph_all)

    # Morph a single pair of models and generate a report
    parser_one = subparsers.add_parser('one')
    parser_one.add_argument('source_model', type=mesh_fname,
        help="Path to source model in OBJ/STL format")
    parser_one.add_argument('target_model', type=mesh_fname,
        help="Path to target model in OBJ/STL format")
    parser_one.set_defaults(func=morph_one)

    
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    args.func(args)
