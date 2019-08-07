import numpy as np
import binvox_rw
import matplotlib.pyplot as plt
with open('model_normalized.solid2.binvox' , 'rb') as f:
    model = binvox_rw.read_as_3d_array(f)

A = model.data

print (model.dims)
fig = plt.figure()
ax = fig.gca(projection='3d')
ax.voxels(A, edgecolor='k')
plt.show()