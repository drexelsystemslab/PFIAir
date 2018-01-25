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
from itertools import chain


def angleHist(model):
    neighborsGraph = findNeighbors(model)
    angles = neighborsGraph[:, 2]
    # plot with degree lables
    (hist, labels) = np.histogram(angles, bins=np.linspace(0, 2 * math.pi, 20), density=True)
    labels = labels[1:20] * 180 / 3.14  # strip out the first label (==0) and convert from degrees to radians
    #print(hist.shape)
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

    #draw the model with colorized faces
    for group in facets:
        color = trimesh.visual.random_color()
        if(type(group) == np.int32):
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
    #labels = labels / (largestBounds[0] * largestBounds[1])
    #plt.bar(labels[:-1], hist, width=0.05)
    #plt.show()
    descriptor = []
    for bin in range(0, len(hist)):
        descriptor.append([labels[bin], hist[bin]])

    return {"faceAreaHist": descriptor}


def localNeighborhoods(model):
    start_point = sample.sample_surface(model,1)
    print(start_point)
    groups = grouping.clusters(model.vertices,0.1)
    print(len(model.vertices))
    sum = 0
    for group in groups:
        sum += len(group)
        color = trimesh.visual.random_color()
        if(type(group) == np.int32):
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


def P_face(verts):
    P_0 = np.array([np.outer(verts[0], verts[0].T), verts[0], 1])
    P_1 = np.array([np.outer(verts[1], verts[1].T), verts[1], 1])
    P_2 = np.array([np.outer(verts[2], verts[2].T), verts[2], 1])
    return np.sum((P_0, P_1, P_2), 0)


def R_face(normal):
    return np.array([np.outer(normal, normal.T), -normal, 1])


def P_n_d(P, n, d):
    return np.dot(np.dot(n.T, P[0]), n) + 2 * d * np.dot(P[1][:, None].T, n) + P[2] * d ** 2


def E_fit(p_array, face_adjacency):
    E_fits = []
    for adjacents in face_adjacency:
        P_e = np.sum((p_array[adjacents[0]], p_array[adjacents[1]]), 0)
        Z = P_e[0] - (np.outer(P_e[1], P_e[1].T) / P_e[2])
        eigenValues, eigenVectors = np.linalg.eig(Z)
        n = eigenVectors[np.argmin(eigenValues)][:, None]
        d = np.dot(n.T, P_e[1][:, None]) / P_e[2]

        E_fit = P_n_d(P_e, n, d) / P_e[2]
        E_fits.append(E_fit[0])
    return E_fits


def faceClustering(model):
    p_array = []
    r_array = []
    face_area_array = model.area_faces

    for i in range(0, len(model.faces)):
        p_array.append(P_face(model.vertices[model.faces[i]]))
        r_array.append(R_face(model.face_normals[i]))

    E_fit_array = E_fit(p_array, model.face_adjacency)

    dual_graph = nx.Graph()  # keep record of graph to guide later edge contraction
    print(np.hstack((model.face_adjacency.astype("object"), E_fit_array)))
    dual_graph.add_weighted_edges_from(np.hstack((model.face_adjacency.astype("object"), E_fit_array)))

    print(model.face_adjacency)
    print(len(face_area_array))

    contraction_graph = nx.DiGraph()
    print(model.faces)
    contraction_graph.add_nodes_from(range(0,len(model.faces)))

    counter = 0
    while (dual_graph.number_of_nodes() > 1):
    # while (counter < 5):
        print(counter)
        edge_to_contract = min(dual_graph.edges(data=True), key=lambda edge: edge[2]['weight'])  # find edge to contract, which connects face a to face b

        a = edge_to_contract[0]
        b = edge_to_contract[1]

        p_prime = p_array[a] + p_array[b]
        r_prime = (face_area_array[a] * r_array[a] + face_area_array[b] * r_array[b]) / (
                    face_area_array[a] + face_area_array[b])

        p_array[a] = p_prime
        p_array[b] = p_prime
        r_array[a] = r_prime
        r_array[b] = r_prime
        face_area_array[a] = face_area_array[a] + face_area_array[b]

        faces = []
        for neighbor in dual_graph.edges(a):  # find all edges where one of the verticies is a
            faces.append(neighbor)#don't need to check if a==b because we are preventing the creation of self loops when we contract
        for neighbor in dual_graph.edges(b):  # find all edges where one of the verticies is b and concat with previous list
            faces.append(neighbor)

        e_fit_prime = E_fit(p_array, faces)
        dual_graph.add_weighted_edges_from(np.hstack((faces, e_fit_prime)))
        # print(np.hstack((faces, e_fit_prime)))


        contraction_graph.add_edge(a,b)
        dual_graph = nx.contracted_nodes(dual_graph, a, b,self_loops=False)

        # model.visual.face_colors[list(nx.descendants(contraction_graph,a))] = trimesh.visual.random_color()

        # print(dual_graph.number_of_nodes())
        # print(a)
        # print(b)
        # model.show(smooth=False)
        counter = counter + 1
    nx.draw(contraction_graph)
    plt.show()
    # model.show(smooth=False)


    # while(graph.number_of_nodes() > 1):
    #     next_to_merge = min(graph.)

    return []
