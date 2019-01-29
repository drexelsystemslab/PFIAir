#!/usr/bin/env python
"""
Quick little script for manually converting .binvox -> .vdb files
"""
import argparse

def filetype(extension):
    def check_file(s):
        if s.endswith(extension):
            return s
        else:
            raise argparse.ArgumentTypeError(
                '\'{}\' must be a {} file'.format(s, extension))
    return check_file

def main(args):
    from binvox2vdb import BinvoxConverter
    converter = BinvoxConverter(args.in_file)
    converter.convert(args.out_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('in_file', type=filetype('.binvox'), 
        help='input file in .binvox format')
    parser.add_argument('out_file', type=filetype('.vdb'),
        help='output file in .vdb format')
    args = parser.parse_args()
    main(args)
