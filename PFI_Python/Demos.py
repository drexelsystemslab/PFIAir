from pfitoolbox import ToolBox
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import trimesh

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
    tree = ToolBox.svd_feature_decomp(model, 0)
    print(tree)
    empty = trimesh.creation.uv_sphere()
    empty = walkthetree(tree,empty)
    empty.show()


def walkthetree(leaf,model):
    if type(leaf) == trimesh.base.Trimesh:
        print(leaf)
        # color = trimesh.visual.random_color()
        leaf.visual.face_colors = trimesh.visual.random_color()


        return model + leaf
    else:
        for l in leaf:
           model = model + walkthetree(l,model)
        return model

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

def random_splitter(model):
    sections = ToolBox.random_splitter(model)
    for section in sections:
        model.visual.face_colors[section] = trimesh.visual.random_color()

    model.show()