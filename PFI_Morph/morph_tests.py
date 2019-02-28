#!/usr/bin/env python
from __future__ import print_function
import argparse

from pfimorph import util, morph, reports

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

def binvox_fname(fname):
    """
    Argparse type function for picking a binvox file
    """
    if fname.endswith('.binvox'):
        return fname
    else:
        raise argparse.ArgumentTypeError(
            "'{}': Filename must end in .binvox".format(fname))

def morph_all(args):
    models = util.get_models()
    # For quick runs, limit the number of models
    if args.model_limit > 0:
        models = models[:args.model_limit]

    # Preprocess the models all at once and rely on the caching to sort
    # out the models. This prevents preprocessing the same model 4 times
    # in parallel once we start the task pool
    print("Preprocessing Models:")
    for binvox_file in models:
        _ = util.binvox_to_vdb(binvox_file)

    # Pair uup the models
    indices, model_pairs = util.get_model_pairs(models)

    N = len(models)
    print("Start morphing {0}x{0} models ({1} morph pairs)...".format(
        N, len(model_pairs)))

    # Run the morph
    stat_pairs = morph.morph_all_pairs(args, model_pairs)

    # Generate N reports
    print("Morphing done! Generating reports...") 
    table = reports.make_stat_table(N, indices, stat_pairs)
    reports.write_many_reports(models, table)

def morph_one(args):
    """
    morph args.source_model into args.target_model
    """
    print("Morphing {} <-> {}".format(args.source_model, args.target_model))
    stat_pair = morph.morph_pair(args, (args.source_model, args.target_model))

    # Save a report
    source_name = util.get_short_name(args.source_model)
    target_name = util.get_short_name(args.target_model)
    report_fname = "output/reports/{}_{}.html".format(source_name, target_name)
    reports.write_report(report_fname, [stat_pair]) 

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--profile', action='store_true',
        help="Set this flag to time each step of the program")
    parser.add_argument('-s', '--save-debug-models', action='store_true',
        help="Set this flag to save extra .obj and .vdb models")
    parser.add_argument(
        '-i', '--iter-limit', type=int, default=20, 
        help="Set max number of frames for each morph (default 20)")
    parser.add_argument(
        '-m', '--model-limit', type=int, default=0,
        help="if set, only morph this many models")

    subparsers = parser.add_subparsers()
    subparsers.required = True
    
    # Morph all pairs of models
    parser_all = subparsers.add_parser('all')
    parser_all.set_defaults(func=morph_all)

    # Morph a single pair of models and generate a report
    parser_one = subparsers.add_parser('one')
    parser_one.add_argument('source_model', type=binvox_fname,
        help="Path to source model in .binvox format")
    parser_one.add_argument('target_model', type=binvox_fname,
        help="Path to target model in .binvox format")
    parser_one.set_defaults(func=morph_one)

    
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    args.func(args)
