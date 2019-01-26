#!/usr/bin/env python
from __future__ import print_function

def main():
    fname = 'output/convert_voxels/bookshelf.binvox'
    print(parse_binvox(fname))

def parse_binvox(fname):
    """
    Parse a binary voxel
    """ 
    with open(fname, 'r') as f:
        # File header
        f.readline()
        
        # dimensions
        dims = f.readline().strip() 
        dims = dims.split(' ')[1:]
        d, w, h = [int(x) for x in dims]

        # Translation
        translate = f.readline().strip()
        translate = translate.split(' ')[1:]
        translate = [float(x) for x in translate]

        # Scale
        scale = f.readline().strip()
        scale = float(scale.split(' ')[1])

        # start of data line
        f.readline()

        vals = []

        while True:
            val_str = f.read(1) 
            if not val_str:
                break
            length_str = f.read(1)
            
            val = ord(val_str)
            length = ord(length_str)

            vals.append((val, length))
            
        return {
            'dims': (d, w, h),
            'translate': tuple(translate),
            'scale': scale,
            'vals': vals
        }

if __name__ == "__main__":
    main()
