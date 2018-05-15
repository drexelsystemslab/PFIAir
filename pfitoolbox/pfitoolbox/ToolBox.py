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


def P_face(verts):
    P_0 = np.array([np.outer(verts[0], verts[0].T), verts[0], 1])
    P_1 = np.array([np.outer(verts[1], verts[1].T), verts[1], 1])
    P_2 = np.array([np.outer(verts[2], verts[2].T), verts[2], 1])
    return np.sum((P_0, P_1, P_2), 0)


def R_face(normal):
    return np.array([np.outer(normal, normal.T), -normal, 1])


def P_n_d(P, n, d):
    return np.dot(np.dot(n.T, P[0]), n) + 2 * d * np.dot(P[1][:, None].T, n) + P[2] * d ** 2


def R_n(R, n):
    return np.dot(np.dot(n.T, R[0]), n) + 2 * np.dot(R[1][:, None].T, n) + R[2]


def exterior_edge(merging_faces_edges):
    vals, inverse, count = np.unique(merging_faces_edges, return_inverse=True, return_counts=True)
    idx_exterior = np.where(count < 2)[0]
    return vals[idx_exterior]


def interior_edge(merging_faces_edges):
    vals, inverse, count = np.unique(merging_faces_edges, return_inverse=True, return_counts=True)
    idx_interior = np.where(count > 1)[0]
    return vals[idx_interior]


def E_shape(irregularity_array, edges_array, edges_length, face_area_array, face_adjacency):
    result = []
    for edge in face_adjacency:
        merging_faces_edges = edges_array[edge[0]] + edges_array[edge[1]]
        ex_edge = exterior_edge(merging_faces_edges)
        p = np.sum(edges_length[0][ex_edge])
        gamma = p ** 2 / (4 * np.pi * (face_area_array[edge[0]] + face_area_array[edge[1]]))
        E_shapes = ((gamma - np.max((irregularity_array[edge[0]], irregularity_array[edge[1]]))) / gamma)
        result.append(0. * E_shapes)  # also returns the face's gamma so it can be used later
    return np.array(result)[:,None]


def irregularity(face_area, verts):
    perimeter = np.linalg.norm(verts[0] - verts[1]) + np.linalg.norm(verts[1] - verts[2]) + np.linalg.norm(
        verts[0] - verts[2])
    return perimeter ** 2 / (4. * np.pi * face_area)


def dist_from_Sphere(vert, center, r):
    return np.square(np.linalg.norm(vert - center, axis=1) - r)


def E_fit_Sphere(face_adjacency, face_vertices):
    verts_face1 = face_vertices[face_adjacency[0]]
    verts_face2 = face_vertices[face_adjacency[1]]
    verts = np.vstack((verts_face1, verts_face2))
    verts = np.unique(verts, axis=0)
    ones = np.ones((verts.shape[0], 1))
    A = np.hstack((2 * verts, ones))
    # print(A)
    b = np.linalg.norm(verts, axis=1) ** 2
    b = b[:, None]
    x = np.dot(A.T, A)
    if np.linalg.cond(x) < 1 / sys.float_info.epsilon:
        w = np.dot(np.linalg.inv(np.dot(A.T, A)), np.dot(A.T, b))
        c_x = w[0, 0]
        c_y = w[1, 0]
        c_z = w[2, 0]
        center = np.array([c_x, c_y, c_z])
        r = np.sqrt(w[3, 0] + c_x ** 2 + c_y ** 2 + c_z ** 2)
        return [np.sum(dist_from_Sphere(verts, center, r))], [c_x, c_y, c_z, r, 0, 0, 0]
    else:
        return [np.inf], [1, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf]


def E_fit_plane(p_array, face_adjacency):
    P_e = np.sum((p_array[face_adjacency[0]], p_array[face_adjacency[1]]), 0)
    Z = P_e[0] - (np.outer(P_e[1][:, None], P_e[1][:, None].T) / P_e[2])
    eigenValues, eigenVectors = np.linalg.eig(Z)
    n = eigenVectors[:, np.argmin(eigenValues)][:, None]
    d = np.dot(-1 * n.T, P_e[1][:, None]) / P_e[2]
    E_fit_plane = P_n_d(P_e, n, d) / P_e[2]
    return E_fit_plane[0], [n[0][0], n[1][0], n[2][0], 0, 0, 0,
                            0]  # TODO: n is temporary # extra [0] to get the value out of the "Tracked array"


def E_dir(r_array, face_area_array, face_adjacency):
    result = []
    for edge in face_adjacency:
        R_e = (face_area_array[edge[0]] * r_array[edge[0]] + face_area_array[edge[1]] * r_array[
            edge[1]]) / (face_area_array[edge[0]] + face_area_array[edge[1]])
        D_e = R_e[0]
        e_e = R_e[1]
        n = -2. * np.dot((np.linalg.pinv(D_e + D_e.T)), e_e)
        n = n / np.linalg.norm(n)
        E_dir = R_n(R_e, n)
        result.append(E_dir[0])
    return np.array(result)[:,None]


def get_face_adj_info(model, edges_array, edges_length, edges_vector_norm2, face_center):
    interior_edges = []
    edges_array = np.asarray(edges_array)
    faces_edges = np.hstack((edges_array[model.face_adjacency[:, 0]], edges_array[model.face_adjacency[:, 1]]))
    for i in range(model.face_adjacency.shape[0]):
        vals, counts = np.unique(faces_edges[i], return_counts=True)
        interior_edges.append(vals[np.where(counts > 1)][0])
    interior_edges = np.asarray(interior_edges)[:, None]
    adjacent_interior = np.hstack((model.face_adjacency, interior_edges))
    Normals = np.hstack((model.face_normals[model.face_adjacency[:, 0]].reshape(-1, 1, 3),
                         model.face_normals[model.face_adjacency[:, 1]].reshape(-1, 1, 3)))
    angle = np.arccos(np.clip(np.sum(Normals[:, 0, :] * Normals[:, 1, :], axis=1), -1.0, 1.0))[:, None]
    FC = face_center[model.face_adjacency[:, 1]] - face_center[model.face_adjacency[:, 0]]
    FC_Length = np.linalg.norm(FC, axis=1).reshape(-1, 1)
    FC = FC / FC_Length
    cosine_u_w = np.sum(Normals[:, 0, :] * FC, axis=1)
    convexity = np.where(cosine_u_w <= 0, 1, -1)[:, None]
    angle = angle * convexity
    edges_vector_norm2 = edges_vector_norm2[interior_edges, :]
    edges_vector_norm2 = edges_vector_norm2.reshape((-1, 3))
    face_adjacency_info = np.hstack(
        (model.face_adjacency, interior_edges, edges_length[0][interior_edges], edges_vector_norm2, angle))
    face_adjacency_info = face_adjacency_info[face_adjacency_info[:, 2].argsort()]
    # print (face_adjacency_info)
    return face_adjacency_info


def calculate_COV(int_edge, face_adjacency_info, cov):
    for e in int_edge:
        e_info = face_adjacency_info[e]
        cov = cov + e_info[3] * e_info[7] * np.dot(e_info[4:7].reshape(-1, 1), e_info[4:7].reshape(1, -1))
    return cov


def get_centroids(leaves, face_area, face_center):
    face_area = np.asarray(face_area)
    centroid = np.average(face_center[leaves],
                          axis=0,
                          weights=face_area[leaves])
    return centroid


def best_fitting_circle(v):
    d = v.shape[0]
    ones = np.ones((d, 1))
    A = np.hstack((2 * v, ones))
    b = v[:, 0] ** 2 + v[:, 1] ** 2
    b = b[:, None]
    x = np.dot(A.T, A)
    if np.linalg.cond(x) < 1 / sys.float_info.epsilon:
        w = np.dot(np.linalg.inv(np.dot(A.T, A)), np.dot(A.T, b))
        c_x = w[0, 0]
        c_y = w[1, 0]
        center = np.array([c_x, c_y])
        r = np.sqrt(w[2, 0] + c_x ** 2 + c_y ** 2)
        return center, r
    else:
        raise AssertionError("No Cylinder fitted")


def fit_cylinder(cov, leaves, face_area, face_center, vertices):
    # print (np.argmin(eigenValues))
    try:
        eigenValues, eigenVectors = np.linalg.eig(cov)
        if not np.all(np.isreal(eigenValues)):
            raise AssertionError("complex eigen values and vectors for cylinder")
        else:
            n = eigenVectors[:, np.argmin(eigenValues)][:, None]
            temp = list(range(3))
            temp.pop(np.argmin(eigenValues))
            e_x = eigenVectors[:, temp[0]][:, None]
            e_y = eigenVectors[:, temp[1]][:, None]
            cm = get_centroids(leaves, face_area, face_center)
            v_projected = np.hstack(
                (np.dot(vertices - cm, e_x)[:, None], (np.dot(vertices - cm, e_y)[:, None]))).reshape(
                (-1, 2))
            c, r = best_fitting_circle(v_projected)
            c_cylinder = cm[:, None] + c[0] * e_x + c[1] * e_y
            return c_cylinder, r, n
    except AssertionError as e:
        raise e


def find_leaves(contraction_graph, adjacent, n):
    if adjacent[0] < n:
        leaves = [adjacent[0]]
    elif adjacent[0] >= n:
        all_decendants = list(nx.descendants(contraction_graph, adjacent[0]))
        leaves = [i for i in all_decendants if i < n]
    if adjacent[1] < n:
        leaves = leaves + [adjacent[1]]
    elif adjacent[1] >= n:
        all_decendants = list(nx.descendants(contraction_graph, adjacent[0]))
        leaves = leaves + [i for i in all_decendants if i < n]
    return leaves


def E_fit_cylinder(face_adjacency, face_vertices, cov_list, face_adjacency_info, edges_array, face_area, face_center,
                   contraction_graph, n):
    merging_faces_edges = edges_array[face_adjacency[0]] + edges_array[face_adjacency[1]]
    int_edge = interior_edge(merging_faces_edges)
    cov_prime = cov_list[face_adjacency[0]] + cov_list[face_adjacency[1]]
    cov_prime = calculate_COV(int_edge, face_adjacency_info, cov_prime)
    merging_vertices = np.vstack((face_vertices[face_adjacency[0]], face_vertices[face_adjacency[1]]))
    merging_vertices = np.unique(merging_vertices, axis=0)
    leaves = find_leaves(contraction_graph, face_adjacency, n)
    try:
        c_cylinder, r, c_cylinder_normal = fit_cylinder(cov_prime, leaves, face_area, face_center, merging_vertices)
        c_cylinder = c_cylinder.reshape((1, 3))
        A = merging_vertices - c_cylinder
        c_cylinder_normal = c_cylinder_normal.reshape((1, 3))
        cross = np.linalg.norm(np.cross(A, c_cylinder_normal), axis=1)
        return np.sum((cross - r) ** 2), [c_cylinder[0][0], c_cylinder[0][1], c_cylinder[0][2], c_cylinder_normal[0][0],
                                          c_cylinder_normal[0][1], c_cylinder_normal[0][2], r]
    except AssertionError:
        return np.inf, [2, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf]


def fit_shapes(face_adjacency, p_array, face_vertices, cov_list, face_adjacency_info, edges_array, face_area_array,
               face_center, contraction_graph, n):
    result = []
    for edge in face_adjacency:
        E_fit_array, fitted_plane = E_fit_plane(p_array, edge)
        E_sphere, fitted_sphere = E_fit_Sphere(edge, face_vertices)
        E_fit_cylinders, fitted_cylinder = E_fit_cylinder(edge, face_vertices, cov_list, face_adjacency_info,
                                                          edges_array,
                                                          face_area_array, face_center, contraction_graph, n)
        result.append(np.hstack((E_fit_array, E_sphere, E_fit_cylinders, fitted_plane, fitted_sphere, fitted_cylinder)))
    return np.array(result)


def E_fit(shape_fits):
    return np.min(shape_fits,axis=1)[:,None]


def E(r_array, face_area_array, face_adjacency, edges_array, edges_length, irregularity_array, p_array, face_vertices,
      cov_list, face_adjacency_info, face_center, contraction_graph, n):
    result = []


    edir = E_dir(r_array, face_area_array, face_adjacency)
    eshape = E_shape(irregularity_array, edges_array, edges_length, face_area_array, face_adjacency)

    efitshapes = fit_shapes(face_adjacency, p_array, face_vertices, cov_list, face_adjacency_info,
                                     edges_array, face_area_array, face_center, contraction_graph, n)


    efit = E_fit(efitshapes)


    return np.hstack((efit,edir,eshape,efitshapes))


def faceClustering(model):
    p_array = []
    r_array = []
    irregularity_array = []
    face_area_array = list(model.area_faces)
    edges_length = np.linalg.norm(
        model.vertices[model.edges_unique][:, 1, :] - model.vertices[model.edges_unique][:, 0, :], axis=1)
    l = edges_length.reshape((len(edges_length), 1))
    edges_length = edges_length.reshape((1, len(edges_length)))
    edges_vector_norm2 = (model.vertices[model.edges_unique][:, 1, :] - model.vertices[model.edges_unique][:, 0, :]) / l
    face_vertices = []
    cov_list = []
    face_center = model.triangles_center

    e_array = []

    for i in range(0, len(model.faces)):
        A = model.vertices
        # print(model.vertices[model.faces[i]])
        p_array.append(P_face(model.vertices[model.faces[i]]))
        r_array.append(R_face(model.face_normals[i]))
        irregularity_array.append(irregularity(face_area_array[i], model.vertices[model.faces[i]]))
        face_vertices.append(model.vertices[model.faces[i]])
        cov_list.append(np.zeros((3, 3)))

    edges_array = model.faces_unique_edges.tolist()

    contraction_graph = nx.DiGraph()
    contraction_graph.add_nodes_from(range(0, len(model.faces)))
    nx.set_node_attributes(contraction_graph, [0, 0, 0, 0, 0, 0, 0, 0], "fitted_shape")
    counter = 0
    Normals = model.face_normals
    face_adjacency_info = get_face_adj_info(model, edges_array, edges_length, edges_vector_norm2, face_center)

    n = len(model.faces)
    E_array = E(r_array, face_area_array, model.face_adjacency, edges_array, edges_length, irregularity_array,
                p_array, face_vertices, cov_list, face_adjacency_info, face_center, contraction_graph, n)
    dual_graph = nx.Graph()

    dual_graph.add_weighted_edges_from(
        np.hstack((model.face_adjacency.astype("object"), np.sum(E_array[:, 0:3], axis=1)[:, None])))

    plane_or_sphere_data = np.hstack((model.face_adjacency, np.argmin(E_array[:, 3:6], axis=1)[:, None]))
    map(lambda face: dual_graph.add_edge(face[0], face[1], plane_or_sphere=[face[2], 0, 0, 0, 0, 0, 0, 0]),
        plane_or_sphere_data)

    while (dual_graph.number_of_nodes() > 1):
        # find edge to contract, which connects face a to face b
        edge_to_contract = min(dual_graph.edges(data=True), key=lambda edge: edge[2]['weight'])

        # all three arrays should be the same length, so it shouldn't matter which one we choose
        face_prime = len(p_array)
        a = int(edge_to_contract[0])
        b = int(edge_to_contract[1])

        contraction_graph.add_node(face_prime)
        contraction_graph.add_edge(face_prime, a)
        contraction_graph.add_edge(face_prime, b)
        all_decendants = list(nx.descendants(contraction_graph, face_prime))
        leaves = [i for i in all_decendants if i < len(model.faces)]
        colors = [[229, 156, 11, 255], [11, 204, 229, 255], [226, 30, 9, 255]]  # plane, sphere, cylinder
        model.visual.face_colors[leaves] = trimesh.visual.random_color()
        model.visual.face_colors[leaves] = colors[int(dual_graph.get_edge_data(a, b)["plane_or_sphere"][0])]

        p_prime = p_array[a] + p_array[b]
        r_prime = (face_area_array[a] * r_array[a] + face_area_array[b] * r_array[b]) / (
                face_area_array[a] + face_area_array[b])
        p_array.append(p_prime)
        r_array.append(r_prime)
        face_area_array.append(face_area_array[a] + face_area_array[b])
        merging_faces_edges = edges_array[a] + edges_array[b]
        ex_edge = exterior_edge(merging_faces_edges)
        int_edge = interior_edge(merging_faces_edges)
        cov_prime = cov_list[a] + cov_list[b]
        cov_prime = calculate_COV(int_edge, face_adjacency_info, cov_prime)
        cov_list.append(cov_prime)

        edges_array.append(ex_edge.tolist())
        new_face_vertices = np.vstack((face_vertices[a], face_vertices[b]))
        new_face_vertices = np.unique(new_face_vertices, axis=0)
        face_vertices.append(new_face_vertices)

        dual_graph.add_node(face_prime)

        dual_graph = nx.contracted_nodes(dual_graph, face_prime, a, self_loops=False)
        dual_graph = nx.contracted_nodes(dual_graph, face_prime, b, self_loops=False)
        edges_prime = list(dual_graph.edges(face_prime))

        if len(edges_prime) > 0:
            edges_prime = np.unique(edges_prime, axis=0)

            merging_faces_edges = edges_array[a] + edges_array[b]
            ex_edge = exterior_edge(merging_faces_edges)
            p = np.sum(edges_length[0][ex_edge])
            gamma = p ** 2 / (4 * np.pi * (face_area_array[a] + face_area_array[b]))
            irregularity_array.append(gamma)

            E_prime = E(r_array, face_area_array, edges_prime, edges_array, edges_length, irregularity_array,
                        p_array, face_vertices, cov_list, face_adjacency_info, face_center, contraction_graph, n)

            dual_graph.add_weighted_edges_from(np.hstack(
                (edges_prime.astype("object"),
                 np.resize(np.sum(E_prime[:, 0:3], axis=1), (edges_prime.shape[0], 1)))))

            plane_or_sphere = np.argmin(E_prime[:, 3:6], axis=1)

            fitted_shape = np.zeros((len(edges_prime), 7))
            fitted_shape[plane_or_sphere == 0] = E_prime[plane_or_sphere == 0, 6:13]
            fitted_shape[plane_or_sphere == 1] = E_prime[plane_or_sphere == 1, 13:20]
            fitted_shape[plane_or_sphere == 2] = E_prime[plane_or_sphere == 2, 20:27]
            plane_or_sphere_data = np.hstack((edges_prime, plane_or_sphere[:, None], fitted_shape))
            map(lambda face: dual_graph.add_edge(face[0], face[1], plane_or_sphere=face[2:]), plane_or_sphere_data)
        if (counter % 100 == 0):
            print(counter)

        counter = counter + 1

    # pos = hierarchy_pos(contraction_graph, len(p_array) - 1)
    # nx.draw(contraction_graph, pos=pos, with_labels=True)
    #
    # plt.show()
    # model.show(smooth=False)

    # while(graph.number_of_nodes() > 1):
    #     next_to_merge = min(graph.)

    return contraction_graph


def hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5,
                  pos=None, parent=None):
    '''If there is a cycle that is reachable from root, then this will see infinite recursion.
       G: the graph
       root: the root node of current branch
       width: horizontal space allocated for this branch - avoids overlap with other branches
       vert_gap: gap between levels of hierarchy
       vert_loc: vertical location of root
       xcenter: horizontal location of root
       pos: a dict saying where all nodes go if they have been assigned
       parent: parent of this branch.'''
    if pos == None:
        pos = {root: (xcenter, vert_loc)}
    else:
        pos[root] = (xcenter, vert_loc)
    neighbors = list(G.neighbors(root))
    if len(neighbors) != 0:
        dx = width / len(neighbors)
        nextx = xcenter - width / 2 - dx / 2
        for neighbor in neighbors:
            nextx += dx
            pos = hierarchy_pos(G, neighbor, width=dx, vert_gap=vert_gap,
                                vert_loc=vert_loc - vert_gap, xcenter=nextx, pos=pos,
                                parent=root)
    return pos


def contraction_graph_to_seg(contraction_graph, model):
    root = len(contraction_graph) - 1  # makes the assumtion that the last node added is the root
    clusters = []
    for node in contraction_graph[root].keys():
        for node2 in contraction_graph[node].keys():
            for node3 in contraction_graph[node2].keys():
                clusters.append(node3)
    seg = np.zeros((len(model.faces), 1)).astype("int")
    for i, cluster in enumerate(clusters):
        leaves = [j for j in list(nx.descendants(contraction_graph, cluster)) if j < len(model.faces)]
        seg[leaves] = i
    return seg
