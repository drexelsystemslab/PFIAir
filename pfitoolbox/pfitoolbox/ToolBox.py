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
import scipy



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

def randomWalkerSegmentation(model):
    #prep the edge graph
    g = model.vertex_adjacency_graph
    pairs = model.face_adjacency
    face_normal_pairs = model.face_normals[pairs]
    weights = np.linalg.norm(face_normal_pairs[:,0]-face_normal_pairs[:,1],axis=1)
    edge_weights = np.hstack((pairs,weights[:,None]))
    g.add_weighted_edges_from(edge_weights)

    # find the farthest point from the centroid
    centroid = model.centroid
    s1 = np.argmax(scipy.spatial.distance.cdist(centroid[None, :], model.triangles_center))
    s = [s1]
    # find the rest of s iteratively
    D_n_minus_1 = 0#start with zero to ensure it runs at least twice
    d_D_n_minus_1 = 0
    while (True):
        paths = np.zeros((len(model.faces),1))
        for s_i in s:
            path_length_dict = nx.single_source_dijkstra_path_length(g,s_i)
            temp = np.zeros((len(path_length_dict),1))
            for face, path_length in path_length_dict.iteritems():
                paths[int(face),-1]=path_length
            paths = np.hstack((paths,np.zeros((len(model.faces),1))))
        min_1_n_D = np.argmin(paths[:,:-1],axis=1)
        arg_max = np.argmax(np.min(paths[:,:-1],axis=1))
        f_k_D = paths[arg_max,min_1_n_D[arg_max]]
        print(f_k_D)
        if(D_n_minus_1 != 0):
            print(((f_k_D - D_n_minus_1)-d_D_n_minus_1)/(d_D_n_minus_1))
        if(D_n_minus_1 != 0 and ((f_k_D - D_n_minus_1)-d_D_n_minus_1)/(d_D_n_minus_1) < -50):#if there was a significant decrease in D over D_n-1
            break
        else:
            d_D_n_minus_1 = D_n_minus_1-f_k_D
            D_n_minus_1 = f_k_D
            s.append(arg_max)
            print(arg_max)

    for facet in s:
        model.visual.face_colors[facet] = trimesh.visual.random_color()
    model.show()

    adjancency = nx.adjacency_matrix(g).todense()
    normalized_adanceny = adjancency/adjancency.sum(axis=1)

    print(normalized_adanceny)
    print("test")
    first_crossing = fmpt(normalized_adanceny)

    print(s)

    #print(first_crossing[:,s])
    max_p = np.argmin(np.abs(first_crossing[:,s]),axis=1)
    print(max_p)
    print(np.where(max_p == 1))
    for group in range(0,len(s)):
        color = trimesh.visual.random_color()
        print(len(np.where(max_p == group)[0]))
        for facet in np.where(max_p == group)[0]:
            model.visual.face_colors[facet] = color
    model.show()

    #print(first_crossing)
    #print(np.sum(normalized_adanceny,axis=1))




    return

def steady_state(P):
    # https://github.com/jmmcd/GPDistance/blob/master/python/RandomWalks/ergodic.py

    v,d=np.linalg.eig(np.transpose(P))

    # for a regular P maximum eigenvalue will be 1
    mv=max(v)
    # find its position
    i=v.tolist().index(mv)

    # normalize eigenvector corresponding to the eigenvalue 1
    return d[:,i]/sum(d[:,i])


def fmpt(P):
    #https://github.com/jmmcd/GPDistance/blob/master/python/RandomWalks/ergodic.py
    A = np.zeros_like(P)
    ss = steady_state(P)
    k = ss.shape[0]
    for i in range(k):
        A[:, i] = ss
    A = A.transpose()
    I = np.identity(k)
    Z = np.linalg.inv(I - P + A)
    E = np.ones_like(Z)
    D = np.diag(1. / np.diag(A))
    Zdg = np.diag(np.diag(Z))
    M = (I - Z + E * Zdg) * D
    return M