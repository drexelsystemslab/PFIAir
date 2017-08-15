from pfitoolbox import ToolBox
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np

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
    #model.visual.face_colors[m_n[1]] = [66, 134, 244, 255]
    model.show()

def svd_with_distance_maps(model):
    dists = distance_map(model)
    plt.imshow(dists, interpolation='nearest', cmap=cm.gist_rainbow)
    plt.figure()

    U, s, V = np.linalg.svd(dists, full_matrices=True)
    first_order_s = np.zeros_like(dists)
    print(first_order_s.shape)
    first_order_s[0, 0] = s[0]
    first_order_dists = np.dot(U, np.dot(first_order_s, V))
    first_order_diffs = np.absolute(first_order_dists - dists)
    plt.imshow(first_order_diffs, interpolation='nearest', cmap=cm.gist_rainbow)
    plt.figure()

    second_order_s = np.zeros_like(dists)
    second_order_s[1, 1] = s[1]
    second_order_dists = np.dot(U, np.dot(second_order_s, V))
    second_order_diffs = np.absolute(second_order_dists - dists)
    plt.imshow(second_order_diffs, interpolation='nearest', cmap=cm.gist_rainbow)
    plt.figure()

    closest = np.less(first_order_diffs, second_order_diffs)
    plt.imshow(closest, interpolation='nearest', cmap=cm.tab20)

    plt.show()
