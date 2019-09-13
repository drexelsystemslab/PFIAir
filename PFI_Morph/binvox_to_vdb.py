#!/usr/bin/env python
"""
Quick little script for manually converting .binvox -> .vdb files
"""
import os
import argparse

def filetype(extension):
    def check_file(s):
        if s.endswith(extension):
            return s
        else:
            raise argparse.ArgumentTypeError(
                '\'{}\' must be a {} file'.format(s, extension))
    return check_file

def high_res_fname(args):
    """
    turn /path/to/output.vdb -> /path/to/output_hi_res.vdb

    or return None if --hi-res is not specified
    """
    if not args.hi_res:
        return None

    path, out_vdb = os.path.split(args.out_file)
    model_name, _ = os.path.splitext(out_vdb)

    new_fname = '{}_hi_res.vdb'.format(model_name) 
    return os.path.join(path, new_fname)

def main(args):
    from binvox2vdb import BinvoxConverter
    converter = BinvoxConverter(args.in_file)
    converter.convert(args.out_file, high_res_fname=high_res_fname(args))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('in_file', type=filetype('.binvox'), 
        help='input file in .binvox format')
    parser.add_argument('out_file', type=filetype('.vdb'),
        help='output file in .vdb format')
    parser.add_argument('--hi-res', action='store_true',
        help='if specified, save <out_file>_hi_res.vdb too')
    args = parser.parse_args()
    main(args)
