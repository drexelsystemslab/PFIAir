from __future__ import print_function
import numpy as np
import math
import time
import pickle
#from stl import mesh
import matplotlib.pyplot as plt
import sys
import trimesh
import networkx as nx
from trimesh import sample,grouping,geometry
import random



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
    walks = np.random.randint(0,3,(num_of_particles,t_f))#each particle has a row of length equal to the number of time steps
    current_position = np.zeros((num_of_particles,t_f+1),dtype=int)+random.randint(0,len(model.facets))
    #print(current_position)
    # print(walks)
    walker = np.vectorize(lambda a,b:neighbors[a][b])

    starting_points = model.vertices[model.faces[current_position[:, 0]][:,0]]  # arbitrarily choosing vertex 0 to represent the position of the face

    start = time.clock()
    for t in range(0,t_f):
        current_position[:,t+1] = walker(current_position[:,t], walks[:,t])

    end = time.clock()
    print("Time elapsed: %s" % (end-start))

    ending_points = model.vertices[model.faces[current_position[:, -1]][:, 0]]
    distance_travelled = np.linalg.norm(ending_points - starting_points)

    print("Average distance from starting point: %s" % np.average(distance_travelled))


    return current_position.flatten() #return a list of all the nodes that have been visited

def flood_segmentation(model):
    # print(len(model.faces))
    # print(len(model.face_adjacency_projections))
    graph = nx.from_edgelist(model.face_adjacency)


    # print(model.face_adjacency_projections[:,None].shape)
    # print(model.face_adjacency.shape)
    #
    for i,edge in enumerate(model.face_adjacency):
        graph.add_edge(edge[0],edge[1],weight=model.face_adjacency_projections[i])

    groups = {}
    for i in range(0,100000):
        starting_face = random.choice(list(graph.nodes))
        print(starting_face)
        # subgraph_faces = [starting_face]
        # subgraph_edges = []
        for edge in nx.bfs_edges(graph, starting_face):
            print(graph.edges[edge[0],edge[1]]['weight'])
            if(graph.edges[edge[0],edge[1]]['weight']<1):
                graph.nodes[edge[1]]['color'] = starting_face
                #graph.edges[edge[0], edge[1]]['color'] = starting_face
            else:
                print("break")
                break
            graph.edges[edge[0], edge[1]]['weight'] = 0
        # groups.append(subgraph_faces)
        break
    for node in graph.nodes(data='color', default=None):
        if(node[1] != None):
            try:
                groups[node[1]].append(node[0])
            except KeyError:
                groups[node[1]] = [node[0]]
                pass

    return groups
        # print(nx.get_edge_attributes(graph, edge))
    #
    # for edge in nx.bfs_edges(graph,0):
    #     print(type(edge))

    #
    # combined_array = np.hstack((model.face_adjacency,model.face_adjacency_projections[:,None].astype('int')))
    # tuples = tuple([tuple(row) for row in combined_array])
    # print(tuples)
    # graph = nx.Graph(tuples)