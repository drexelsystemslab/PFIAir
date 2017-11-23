from __future__ import print_function
import numpy as np
import math
import time
import pickle
# from stl import mesh
import matplotlib.pyplot as plt
import sys
import trimesh
import networkx as nx
from trimesh import sample, grouping, geometry
import random
import itertools
from scipy import ndimage


def angleHist(model):
    neighborsGraph = findNeighbors(model)
    angles = neighborsGraph[:, 2]
    # plot with degree lables
    (hist, labels) = np.histogram(angles, bins=np.linspace(0, 2 * math.pi, 20), density=True)
    labels = labels[1:20] * 180 / 3.14  # strip out the first label (==0) and convert from degrees to radians
    # print(hist.shape)
    descriptor = []  # append hist and lables such that [[labels[0],hist[0]]
    for bin in range(0, len(hist)):
        descriptor.append([labels[bin], hist[bin]])
    return {"angleHist": descriptor}


def findNeighbors(model):
    adjacents = model.face_adjacency  # returns a list of pairs of faces that share an edge
    angles = trimesh.geometry.vector_angle(
        model.face_normals[adjacents])  # calculate the angle between the normal vectors of each of the neighbors

    N = adjacents.shape
    neighbors = np.zeros((N[0], N[1] + 1))  # increase size of neighbors array from (n,2) to (n,3)
    neighbors[:, :-1] = adjacents  # add the adjacents to this new array
    neighbors[:,
    2] = angles  # add the angle such that for each set of neighbors, neighbors = (neighbor1,neighbor2,angle)
    return neighbors


def faceDetector(model):
    _, groups = trimesh.grouping.group_vectors(model.face_normals, angle=np.radians(0.1))

    facets_area = []
    for group in groups:
        facets_area.append(np.sum(model.area_faces[group]))

    facets_area = np.array(facets_area)

    largestBounds = model.extents[model.extents.argsort()[-2:]]
    print(largestBounds[0] * largestBounds[1] / 10)

    (hist, labels) = np.histogram(facets_area, bins=np.linspace(0, largestBounds[0] * largestBounds[1] / 20, 50),
                                  density=True)
    print(labels)
    plt.bar(labels[:-1], hist, width=0.05)
    plt.axvline(x=largestBounds[0] * largestBounds[1] / 50)
    plt.show()
    # print(facets_area)
    facets = np.delete(groups, np.where(facets_area < largestBounds[0] * largestBounds[1] / 50))
    print(largestBounds[0] * largestBounds[1] / 50)
    (hist, labels) = np.histogram(facets_area[np.where(facets_area > largestBounds[0] * largestBounds[1] / 100)],
                                  bins=np.linspace(0, largestBounds[0] * largestBounds[1], 20))

    labels = labels / (largestBounds[0] * largestBounds[1])
    print(labels)
    print(hist)
    plt.bar(labels[:-1], hist, width=0.05)
    plt.show()
    descriptor = []
    for bin in range(0, len(hist)):
        descriptor.append([labels[bin], hist[bin]])

    # draw the model with colorized faces
    for group in facets:
        color = trimesh.visual.random_color()
        if (type(group) == np.int32):
            model.visual.face_colors[group] = color

        else:
            for facet in group:
                model.visual.face_colors[facet] = color
    model.show()
    return facets


def faceAreaHist(model):
    _, groups = trimesh.grouping.group_vectors(model.face_normals, angle=np.radians(0.1))

    facets_area = []
    for group in groups:
        facets_area.append(np.sum(model.area_faces[group]))

    facets_area = np.array(facets_area)

    largestBounds = model.extents[model.extents.argsort()[-2:]]
    (hist, labels) = np.histogram(facets_area[np.where(facets_area > largestBounds[0] * largestBounds[1] / 100)],
                                  bins=np.linspace(0, largestBounds[0] * largestBounds[1], 20))
    # labels = labels / (largestBounds[0] * largestBounds[1])
    # plt.bar(labels[:-1], hist, width=0.05)
    # plt.show()
    descriptor = []
    for bin in range(0, len(hist)):
        descriptor.append([labels[bin], hist[bin]])

    return {"faceAreaHist": descriptor}


def localNeighborhoods(model):
    start_point = sample.sample_surface(model, 1)
    print(start_point)
    groups = grouping.clusters(model.vertices, 0.1)
    print(len(model.vertices))
    sum = 0
    for group in groups:
        sum += len(group)
        color = trimesh.visual.random_color()
        if (type(group) == np.int32):
            model.visual.vertex_colors[group] = color

        else:
            for facet in group:
                model.visual.vertex_colors[facet] = color
    print(sum)
    model.show()

    return "test"


def randomWalker(model):
    graph = nx.Graph()
    graph.add_edges_from(model.face_adjacency)
    neighbors = np.array(graph.adjacency_list())
    num_of_particles = 1000
    t_f = 100
    walks = np.random.randint(0, 3, (
        num_of_particles, t_f))  # each particle has a row of length equal to the number of time steps
    current_position = np.zeros((num_of_particles, t_f + 1), dtype=int) + random.randint(0, len(model.facets))
    # print(current_position)
    # print(walks)
    walker = np.vectorize(lambda a, b: neighbors[a][b])

    starting_points = model.vertices[model.faces[current_position[:, 0]][:,
                                     0]]  # arbitrarily choosing vertex 0 to represent the position of the face

    start = time.clock()
    for t in range(0, t_f):
        current_position[:, t + 1] = walker(current_position[:, t], walks[:, t])

    end = time.clock()
    print("Time elapsed: %s" % (end - start))

    ending_points = model.vertices[model.faces[current_position[:, -1]][:, 0]]
    distance_travelled = np.linalg.norm(ending_points - starting_points)

    print("Average distance from starting point: %s" % np.average(distance_travelled))

    return current_position.flatten()  # return a list of all the nodes that have been visited


def three_d_hog(model):
    scale = 30
    cell_size = 2
    temp_model = model.copy()
    transformation, extents = trimesh.bounds.oriented_bounds(temp_model)
    temp_model.apply_transform(transformation)
    temp_model.apply_scale(scale / np.max(extents))  # scale such that the longest side is 10
    print((temp_model.bounds[0] * -1.0) - cell_size)
    temp_model.apply_translation((temp_model.bounds[0] * -1.0) + cell_size)
    bounds = np.array(temp_model.bounds)  # bounds returns by reference, so make a copy
    print(bounds)
    print('test')
    bounds[0, :] = np.subtract(bounds[0, :], 2 * cell_size) + (bounds[0, :] % cell_size)
    bounds[1, :] = np.add(bounds[1, :], 2 * cell_size) - (bounds[1, :] % cell_size)
    bounds = bounds.astype('int')
    print(temp_model.bounds)
    print(bounds)

    shape = (bounds[1, :] - bounds[0, :])
    print(shape)

    indicies = []
    for i in np.arange(bounds[0, 0], bounds[1, 0]):
        for j in np.arange(bounds[0, 1], bounds[1, 1]):
            for k in np.arange(bounds[0, 2], bounds[1, 2]):
                indicies.append((i, j, k))

    distances = trimesh.proximity.signed_distance(temp_model, indicies)
    print(distances)

    distances = np.reshape(distances, shape)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    array_indicies = np.array(indicies)
    filtered_array_indicies = array_indicies[np.ravel(np.abs(distances)) <= cell_size]
    filtered_distances = distances[np.abs(distances) <= cell_size]
    ax.scatter(filtered_array_indicies[:, 0], filtered_array_indicies[:, 1], filtered_array_indicies[:, 2],
               c=filtered_distances / np.max(filtered_distances),
               s=100)
    ax.scatter(temp_model.vertices[:, 0], temp_model.vertices[:, 1], temp_model.vertices[:, 2],
               c=[0, 0, 0], s=100)

    kx = np.array([[[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [1, 0, -2, 0, 1], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]])

    ky = np.array([[[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 1, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, -2, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 1, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]])

    kz = np.array([[[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 1, 0, 0], [0, 0, 0, 0, 0], [0, 0, -2, 0, 0], [0, 0, 0, 0, 0], [0, 0, 1, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
                   [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]])

    dx = ndimage.convolve(distances, kx, mode='constant', cval=0)
    dy = ndimage.convolve(distances, ky, mode='constant', cval=0)
    dz = ndimage.convolve(distances, kz, mode='constant', cval=0)

    dr = np.sqrt(np.power(dx, 2) + np.power(dy, 2) + np.power(dz, 2))
    dtheta = np.arccos(dz / dr) * 180 / np.pi
    dphi = np.arctan2(dx, dy) * 180 / np.pi

    print(np.nanmax(dtheta))
    print(np.nanmin(dtheta))
    print(np.max(dphi))
    print(np.min(dphi))
    print(np.max(dr))
    print(np.min(dr))

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    array_indicies = np.array(indicies)
    filtered_dphi = dphi[np.abs(distances) <= cell_size]
    ax.scatter(filtered_array_indicies[:, 0], filtered_array_indicies[:, 1], filtered_array_indicies[:, 2],
               c=filtered_dphi / np.max(filtered_dphi),
               s=100)
    ax.scatter(temp_model.vertices[:, 0], temp_model.vertices[:, 1], temp_model.vertices[:, 2],
               c=[0, 0, 0], s=100)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    filtered_dtheta = dtheta[np.abs(distances) <= cell_size]
    ax.scatter(filtered_array_indicies[:, 0], filtered_array_indicies[:, 1], filtered_array_indicies[:, 2],
               c=filtered_dtheta / np.max(filtered_dphi),
               s=100)
    ax.scatter(temp_model.vertices[:, 0], temp_model.vertices[:, 1], temp_model.vertices[:, 2],
               c=[0, 0, 0], s=100)

    bins = 3

    cell_start = int(math.floor(cell_size / 2))
    cell_end = int(math.ceil(cell_size / 2))

    phihistograms = []
    thetahistograms = []
    for index in array_indicies:
        phihistograms.append(np.histogram(dphi[index[0] - cell_start:index[0] + cell_end,
                                          index[1] - cell_start:index[1] + cell_end,
                                          index[2] - cell_start:index[2] + cell_end], bins, (-180, 180), density=True)[
                                 0])
        thetahistograms.append(np.histogram(dtheta[index[0] - cell_start:index[0] + cell_end,
                                            index[1] - cell_start:index[1] + cell_end,
                                            index[2] - cell_start:index[2] + cell_end], bins, (0, 180), density=True)[
                                   0])

    phihistograms = np.array(phihistograms)
    thetahistograms = np.array(thetahistograms)
    filtered_array_indicies = array_indicies[np.ravel(np.abs(distances)) <= cell_size]
    filtered_phihistograms = phihistograms[np.ravel(np.abs(distances)) <= cell_size]
    filtered_thetahistograms = thetahistograms[np.ravel(np.abs(distances)) <= cell_size]
    # print(indicies)
    # filtered_indicies = indicies[phihistograms[:,0] == np.nan]
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(filtered_array_indicies[:, 0], filtered_array_indicies[:, 1], filtered_array_indicies[:, 2],
               c=(filtered_phihistograms + np.nanmin(phihistograms)) / (
               np.nanmax(phihistograms) - np.nanmin(phihistograms)),
               s=100)
    ax.scatter(temp_model.vertices[:, 0], temp_model.vertices[:, 1], temp_model.vertices[:, 2],
               c=[0, 0, 0], s=100)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(filtered_array_indicies[:, 0], filtered_array_indicies[:, 1], filtered_array_indicies[:, 2],
               c=(filtered_thetahistograms + np.nanmin(thetahistograms)) / (
                   np.nanmax(thetahistograms) - np.nanmin(thetahistograms)),
               s=100)
    ax.scatter(temp_model.vertices[:, 0], temp_model.vertices[:, 1], temp_model.vertices[:, 2],
               c=[0, 0, 0], s=100)
    plt.show()
