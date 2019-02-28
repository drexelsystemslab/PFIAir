from __future__ import print_function
import os
import json
import multiprocessing
import functools

import lsmorph
from pfimorph import util

def cache_morph(args, fnames):
    """
    If look for a json file for this pair of models and load it
    instead of running the expensive morph code again.

    also, save a JSON file after morphing.

    This is a wrapper function designed to 
    """

    JSON_DIR = 'output/json'
    source_fname, target_fname = fnames
    source_name = util.get_short_name(source_fname)
    target_name = util.get_short_name(target_fname)

    json_fname = "{}/{}-{}.json".format(JSON_DIR, source_name, target_name)
    if os.path.exists(json_fname):
        print("Using cached morph data: {}".format(json_fname))
        with open(json_fname, 'r') as f:
            stat_data = json.load(f)
        stat_pair = lsmorph.StatPair.from_dict(stat_data)
    else:
        stat_pair = morph_pair(args, fnames)
        # Save the stats as a JSON file for later analysis
        stat_pair.save_json(JSON_DIR)

    return stat_pair

def morph_pair(args, fnames):
    """ 
    Morph a single pair of models and return a stat_pair. 

    Filenames must be valid .vdb files
    
    This method is safe to run in multiprocessing.Pool
    """

    # strip off path and extension
    source_fname, target_fname = fnames
    source_name = util.get_short_name(source_fname)
    target_name = util.get_short_name(target_fname)
    
    # Convert models from binvox -> VDB. Note that results are
    # cached.
    source_vdb, source_high_res = util.binvox_to_vdb(source_fname)
    target_vdb, target_high_res = util.binvox_to_vdb(target_fname)

    # Set up the morph
    morpher = lsmorph.Morpher()
    morpher.set_source_info(source_name, source_vdb, source_high_res)
    morpher.set_target_info(target_name, target_vdb, target_high_res)
    
    # Actually run the morph and return the result
    return morpher.morph(
        save_debug_models=args.save_debug_models, 
        profile=args.profile,
        max_iters=args.iter_limit) 

def morph_all_pairs(args, model_pairs):
    """
    Morph all pairs of models from the given list. This uses
    Multiprocessing.pool() to compute the morphs faster.

    returns a list of stat pairs from the morphs
    """
    # Use multiprocessing to morph everything
    func = functools.partial(cache_morph, args)
    cpus = multiprocessing.cpu_count()
    pool = multiprocessing.Pool(cpus)
    stat_pairs = pool.map(func, model_pairs)

    return stat_pairs

