import os
import glob

import trimesh

from binvox2vdb import BinvoxConverter

def is_open_mesh(obj_file):
    """
    Check if a model is closed or open
    """
    mesh = trimesh.io.load.load(obj_file);
    return not mesh.is_watertight

def make_directories(dirname):
    """
    Make a directory if it doesn't already exist
    """
    if not os.path.exists(dirname):
        os.makedirs(dirname)

def stl_to_obj(stl_fname):
    """
    Given an STL filename, convert it to an OBJ file if we haven't
    already. Then return the converted OBJ filename
    """
    CONVERTED_DIR = 'converted_objs'

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

    VDB_DIR = 'output/converted_vdbs'
    make_directories(VDB_DIR)
    return (
        os.path.join(VDB_DIR, vdb_fname),
        os.path.join(VDB_DIR, high_res_fname))

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

    return (vdb_fname, high_res_fname)

def get_short_name(path):
    """
    Strip the path and extension from a filename to get a shorter name
    """
    _, fname = os.path.split(path)
    short_name, _ = os.path.splitext(fname)
    return short_name

def get_models(model_type):
    """
    Get a list of models from one of the data sets
    """
    if model_type == 'closed':
        return glob.glob('princeton/*.obj')
    else:
        return glob.glob('open_mesh_objs/all_pairs/*.obj')

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
