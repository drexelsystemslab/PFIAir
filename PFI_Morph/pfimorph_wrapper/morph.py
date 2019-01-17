import json

import pfimorph
from pfimorph_wrapper import util

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
        with open(json_fname, 'r') as f:
            stat_data = json.load(f)
        stat_pair = pfimorph.StatPair.from_dict(stat_data)
    else:
        stat_pair = morph_pair(args, model_names)
        # Save the stats as a JSON file for later analysis
        stat_pair.save_json(JSON_DIR)

    return stat_pair

def morph_pair(args, fnames):
    """ 
    Morph a single pair of models and return a stat_pair. 

    Filenames must be valid .obj files
    
    This method is safe to run in multiprocessing.Pool
    """
    # strip off path and extension
    source_fname, target_fname = fnames
    source_name = util.get_short_name(source_fname)
    target_name = util.get_short_name(target_fname)

    # Use trimesh to check if models are open/closed
    # by checking if they are watertight
    source_open = util.is_open_mesh(source_fname)
    target_open = util.is_open_mesh(target_fname)

    # Sanity check: does watertight check classify models properly?
    if args.model_type == 'open':
        assert source_open, "{} not open mesh!".format(source_fname)
        assert target_open, "{} not open mesh!".format(target_fname)
    elif args.model_type == 'closed':
        assert not source_open, "{} not closed mesh!".format(source_fname)
        assert not target_open, "{} not closed mesh!".format(target_fname)

    # Set up the morph
    morpher = pfimorph.Morpher()
    morpher.set_source_info(source_fname, source_name, source_open)
    morpher.set_target_info(target_fname, target_name, target_open)
    
    # Actually run the morph and return the result
    return morpher.morph(
        save_debug_models=args.save_debug_models, 
        profile=args.profile,
        max_iters=args.iter_limit) 
