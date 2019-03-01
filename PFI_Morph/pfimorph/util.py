import os
import glob

from binvox2vdb import BinvoxConverter
from pfimorph.config import config

def make_directories(dirname):
    """
    Make a directory if it doesn't already exist
    """
    if not os.path.exists(dirname):
        os.makedirs(dirname)

def vdb_filenames(binvox_fname):
    """
    take the binvox fname /path/to/<model>.binvox

    and turn it into
    (/output/path/<model>.vdb, /output/path/<model>_high_res.vdb)
    """
    path, binvox = os.path.split(binvox_fname)
    model_name, _ = os.path.splitext(binvox)

    vdb_fname = '{}.vdb'.format(model_name)
    high_res_fname = '{}_high_res.vdb'.format(model_name) 

    vdb_dir = config.get('output', 'preprocessed_cache')
    make_directories(vdb_dir)
    return (
        os.path.join(vdb_dir, vdb_fname),
        os.path.join(vdb_dir, high_res_fname))

def binvox_to_vdb(binvox_fname):
    """
    Convert a binvox file to two VDB files: one at normal resolution,
    one at higher resolution for the target model, unless there is a
    a cached copy. In the latter case, just use the converted model
    """
    # Get the names of  the
    vdb_fname, high_res_fname = vdb_filenames(binvox_fname)

    # Only convert if we have not already done so
    if not os.path.exists(vdb_fname) or not os.path.exists(high_res_fname):
        print("Converting {} -> {}".format(binvox_fname, vdb_fname))
        converter = BinvoxConverter(binvox_fname)
        converter.convert(vdb_fname, high_res_fname=high_res_fname)
    else:
        print("Using cached {}".format(vdb_fname))

    return (vdb_fname, high_res_fname)

def get_short_name(path):
    """
    Strip the path and extension from a filename to get a shorter name
    """
    _, fname = os.path.split(path)
    short_name, _ = os.path.splitext(fname)
    return short_name

def get_models():
    """
    Get a list of models from one of the data sets
    """
    binvox_dir = config.get('input', 'binvox_dir')
    glob_pattern = os.path.join(binvox_dir, '*.binvox')
    return glob.glob(glob_pattern)

def get_model_pairs(models):
    """
    Since morphing happens bi-directionally, we only need to iterate
    over the upper triangular portion.
    Let's pre-compute the indexes and model names. Then we can
    use multiprocessing to compute everything in parallel

    This returns 2 lists:
    indices, model_pairs
    indices are of the form [(i, j) ...]
    model_pairs are of the form [(source_fname, target_fname) ...]
    """
    N = len(models)
    indices = []
    model_pairs = [] 
    for i in xrange(N):
        for j in xrange(i, N):
            indices.append((i, j))
            model_pairs.append((models[i], models[j]))
    return indices, model_pairs
