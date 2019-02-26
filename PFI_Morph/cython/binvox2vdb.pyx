# cython: language_level = 2
# distutils: language = c++
from binvox2vdb cimport BinaryToLevelSet, BinvoxData, RunLength

cdef class BinvoxConverter:
    cdef string fname
    cdef float HALF_BANDWIDTH_NORMAL;
    cdef float HALF_BANDWIDTH_HIGH_RES;

    def __init__(self, fname):
        self.fname = fname

        # Cython doesn't seem to support class-level constants. Oh well.
        self.HALF_BANDWIDTH_NORMAL = 3.0
        self.HALF_BANDWIDTH_HIGH_RES = 10.0

    def convert(self, vdb_fname, smoothing_steps=5, high_res_fname=None):
        """
        Convert the binvox file to a vdb file.

        If high_res_fname is specified, this will save a second
        copy with a larger bandwidth
        """
        cdef dict data = self.parse_binvox()
        cdef BinaryToLevelSet converter = BinaryToLevelSet()

        # Unpack the data
        (d, w, h) = data['dims']
        converter.set_dimensions(d, w, h)
        (tx, ty, tz) = data['translate']
        converter.set_translation(tx, ty, tz)
        scale = data['scale']
        converter.set_scale(scale)
        cdef BinvoxData vals = self.convert_pairs(data['vals'])
        converter.populate_grid(vals)

        # Generate the regular .vdb file
        converter.convert(self.HALF_BANDWIDTH_NORMAL, smoothing_steps)
        converter.save(vdb_fname)

        # Generate the high res VDB if desired
        if high_res_fname:
            converter.convert(self.HALF_BANDWIDTH_HIGH_RES, smoothing_steps)
            converter.save(high_res_fname)

    cdef BinvoxData convert_pairs(self, vals):
        """
        Looks like Cython doesn't cast tuples to pairs when
        they're inside a collection. Let's manually convert the vector
        """
        cdef BinvoxData result
        cdef RunLength run
        for value, length in vals:
            run = RunLength(value, length)
            result.push_back(run)
        return result

    cdef dict parse_binvox(self):
        """
        Parse a binary voxel data file
        """ 
        with open(self.fname, 'r') as f:
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
