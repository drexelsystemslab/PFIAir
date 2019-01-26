# cython: language_level = 2
# distutils: language = c++

cdef class BinvoxConverter:
    def __init__(self, dims, translate, scale, vals):
        self.dims = dims
        self.translate = translate
        self.scale = scale
        self.vals = vals
