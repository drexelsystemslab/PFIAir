#from stl import mesh
import time
from pfitoolbox import ToolBox
import tasks
import matplotlib.pyplot as plt
import matplotlib
import trimesh
import numpy as np

from mpl_toolkits import mplot3d

name = 'Coffee_cup'

url = 'models/' + name + '.stl'
fileName = url.split('/')[-1]
print("Generating descriptor for usermodel: " + fileName)
try:
    model = trimesh.load_mesh(url)
except(OSError, IOError, ValueError):
    print("31: stl file missing")
    raise IOError

# ToolBox.faceDetector(model)
# ToolBox.localNeighborhoods(model)
contraction_graph = ToolBox.faceClustering(model)
seg = ToolBox.contraction_graph_to_seg(contraction_graph, model, "segmentations/" + name + ".seg")
# for facet in facets:
#     model.visual.face_colors[facet] = [252, 154, 7, 255]
#
# model.show()
