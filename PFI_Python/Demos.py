from pfitoolbox import ToolBox
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def random_walker(model):
    facets = ToolBox.randomWalker(model)
    for facet in facets:
        model.visual.face_colors[facet] = [252, 154, 7, 255]

    model.show()

def distance_map(model):
    dists = distance_map(model)
    plt.imshow(dists, interpolation='nearest', cmap=cm.gist_rainbow)
    plt.show()

def svd(model):
    m_n = ToolBox.svd_feature_decomp(model)
    model.visual.face_colors[m_n[0]] = [252, 154, 7, 255]
    model.show()
