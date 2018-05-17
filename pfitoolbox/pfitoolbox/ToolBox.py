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


def E_shape(edges_array, edges_length, face_area_array, irregularity_array, face_adjacency):
    perimeter_total = []
    for i in range(len(face_adjacency)):
        merging_faces_edges = edges_array[face_adjacency[i, 0]] + edges_array[face_adjacency[i, 1]]
        ex_edge = exterior_edge(merging_faces_edges)
        perimeter_total.append(np.sum(edges_length[0][ex_edge]))

    perimeter_total = np.asarray(perimeter_total)
    face_area_array = np.asarray(face_area_array).reshape((1, len(face_area_array)))
    area = np.sum(face_area_array[0][face_adjacency], axis=1)
    gamma = perimeter_total ** 2 / (4. * np.pi * area)
    irregularity_array = np.asarray(irregularity_array).reshape((len(irregularity_array), 1))
    gamma_array = np.hstack((irregularity_array[face_adjacency[:, 0]], irregularity_array[face_adjacency[:, 1]]))
    E_shapes = ((gamma - np.max(gamma_array, axis=1)) / gamma)
    return E_shapes


def irregularity(face_area, verts):
    perimeter = np.linalg.norm(verts[0] - verts[1]) + np.linalg.norm(verts[1] - verts[2]) + np.linalg.norm(
        verts[0] - verts[2])
    return perimeter ** 2 / (4. * np.pi * face_area)


def dist_from_Sphere(vert, center, r):
    return np.square(np.linalg.norm(vert - center, axis=1) - r)


def E_fit_Sphere(face_adjacency, face_vertices):
    E_fit_spheres = []
    for adjacent in face_adjacency:
        if adjacent[0] == 35:
            print ("check")
        verts_face1 = face_vertices[adjacent[0]]
        verts_face2 = face_vertices[adjacent[1]]
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
            E_fit_spheres.append([np.sum(dist_from_Sphere(verts, center, r))])
        else:
            E_fit_spheres.append([np.inf])
    return np.array(E_fit_spheres)


def E_fit_plane(p_array, face_adjacency):
    E_fits = []
    i = 0
    for adjacents in face_adjacency:
        P_e = np.sum((p_array[adjacents[0]], p_array[adjacents[1]]), 0)
        Z = P_e[0] - (np.outer(P_e[1][:, None], P_e[1][:, None].T) / P_e[2])
        eigenValues, eigenVectors = np.linalg.eig(Z)
        n = eigenVectors[:, np.argmin(eigenValues)][:, None]
        d = np.dot(-1 * n.T, P_e[1][:, None]) / P_e[2]
        E_fit_plane = P_n_d(P_e, n, d)
        E_fits.append(E_fit_plane[0])  # extra [0] to get the value out of the "Tracked array"
        i = i + 1
    return np.array(E_fits)
def E_fit_plane2(model, face_adjacency, voronoi_area_array, face_vertices_index):
    E_fits = []
    for adjacent in face_adjacency:
        if adjacent[0] == 35:
            print ("checkPl")
        verts_index =  np.unique(np.hstack((face_vertices_index[adjacent[0]], face_vertices_index[adjacent[1]])))
        verts = model.vertices[verts_index]
        # vertex_voronoi = voronoi_area_array[:, adjacent[0]] + voronoi_area_array[:, adjacent[1]]
        # voronoi = vertex_voronoi[verts_index][:,None]
        # s = np.sum(voronoi)
        # v_bar = np.sum(voronoi * verts, axis=0)/s
        v_bar = np.average(verts, axis=0)
        A = verts - v_bar
        # cov = np.matmul(voronoi.reshape(1,-1) * A.T, A)
        cov = np.matmul(A.T,A)
        eigenValues, eigenVectors = np.linalg.eig(cov)
        n = eigenVectors[:, np.argmin(eigenValues)]
        dist2 = (np.dot(A,n)) **2
        # check2  = voronoi * dist2[:,None]
        # E_fit_plane = np.sum(voronoi * dist2[:,None])
        E_fit_plane = np.sum(dist2[:,None])
        E_fits.append(E_fit_plane)
    return np.array(E_fits)[:,None]



def E_dir(r_array, face_area_array, face_adjacency):
    E_dirs = []
    i = 0
    for adjacents in face_adjacency:
        R_e = (face_area_array[adjacents[0]] * r_array[adjacents[0]] + face_area_array[adjacents[1]] * r_array[
            adjacents[1]]) / (
                      face_area_array[adjacents[0]] + face_area_array[adjacents[1]])
        D_e = R_e[0]
        e_e = R_e[1]
        n = -2. * np.dot((np.linalg.pinv(D_e + D_e.T)), e_e)
        n = n / np.linalg.norm(n)
        E_dir = R_n(R_e, n)
        E_dirs.append([E_dir[0]])
        i = i + 1
    return E_dirs


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
            n = eigenVectors[:, np.argmax(eigenValues)][:, None] #change the code to max
            temp = list(range(3))
            temp.pop(np.argmax(eigenValues))
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
    E_fit_cylinders = []
    # calculate_COV(int_edge, face_adjacency_info, cov):
    i = 0
    for adjacent in face_adjacency:
        if adjacent[0] == 35:
            print ("check!")
        merging_faces_edges = edges_array[adjacent[0]] + edges_array[adjacent[1]]
        int_edge = interior_edge(merging_faces_edges)
        cov_prime = cov_list[adjacent[0]] + cov_list[adjacent[1]]
        cov_prime = calculate_COV(int_edge, face_adjacency_info, cov_prime)
        merging_vertices = np.vstack((face_vertices[adjacent[0]], face_vertices[adjacent[1]]))
        merging_vertices = np.unique(merging_vertices, axis=0)
        leaves = find_leaves(contraction_graph, adjacent, n)
        print(i)
        if np.all(np.equal(cov_prime, np.zeros((3,3)))):
            E =np.inf
        else:
            try:
                c_cylinder, r, c_cylinder_normal = fit_cylinder(cov_prime, leaves, face_area, face_center, merging_vertices)
                c_cylinder = c_cylinder.reshape((1, 3))
                A = merging_vertices - c_cylinder
                c_cylinder_normal = c_cylinder_normal.reshape((1, 3))
                cross = np.linalg.norm(np.cross(A, c_cylinder_normal), axis=1)
                E = np.sum((cross - r) ** 2)
            except AssertionError:
                E = np.inf
        i = i + 1
        E_fit_cylinders.append([E])
    return np.asarray(E_fit_cylinders)


def fit_shapes(model, face_adjacency, p_array, face_vertices, cov_list, face_adjacency_info, edges_array, face_area_array,
               face_center, contraction_graph, n, voronoi_area_array, face_vertices_index ):
    E_fit_array = E_fit_plane(p_array, face_adjacency)
    print ('E_plane_1 = ', E_fit_array)
    E_plane = E_fit_plane2(model, face_adjacency, voronoi_area_array, face_vertices_index)
    print('E_plane_2 = ', E_plane)
    E_sphere = E_fit_Sphere(face_adjacency, face_vertices)
    E_fit_cylinders = E_fit_cylinder(face_adjacency, face_vertices, cov_list, face_adjacency_info, edges_array,
                                     face_area_array, face_center, contraction_graph, n)
    return np.hstack((E_plane, E_sphere, E_fit_cylinders))


def E_fit(shape_fits):
    print(shape_fits)
    # return np.around(shape_fits[:,0:3], 25).min(axis=1)[:, None]
    return shape_fits.min(axis=1)[:, None]

def voronoi_area(model):
    result = np.zeros((model.vertices.shape[0], model.faces.shape[0]))
    face_area_array = list(model.area_faces)
    for i in range(0, len(model.faces)):
        result[model.faces[i,0], i] = face_area_array[i]/3.
        result[model.faces[i,1], i] = face_area_array[i]/3.
        result[model.faces[i,2], i] = face_area_array[i]/3.
    print (result)
    return result


def faceClustering(model):
    p_array = []
    r_array = []
    irregularity_array = []
    face_area_array = list(model.area_faces)
    edges_length = np.linalg.norm(model.vertices[model.edges_unique][:, 1, :] - model.vertices[model.edges_unique][:, 0, :], axis=1)
    l = edges_length.reshape((len(edges_length), 1))
    edges_length = edges_length.reshape((1, len(edges_length)))
    edges_vector_norm2 = (model.vertices[model.edges_unique][:, 1, :] - model.vertices[model.edges_unique][:, 0, :]) / l
    face_vertices = []
    face_vertices_index = []
    cov_list = []
    face_center = model.triangles_center
    vertex_neighbors = model.vertex_neighbors[0]
    voronoi_area_array = voronoi_area(model)
    # model.vertex_neighbors[0].keys()
    b = model.faces
    print (type(vertex_neighbors))
    dual_graph = nx.Graph()
    for i in range(0, len(model.faces)):
        A = model.vertices
        print(model.vertices[model.faces[i]])
        p_array.append(P_face(model.vertices[model.faces[i]]))
        r_array.append(R_face(model.face_normals[i]))
        irregularity_array.append(irregularity(face_area_array[i], model.vertices[model.faces[i]]))
        face_vertices.append(model.vertices[model.faces[i]])
        face_vertices_index.append(model.faces[i])
        cov_list.append(np.zeros((3, 3)))
        # dual_graph.add_node(i,degree = 0)
        dual_graph.add_node(i, degree=3)
    # print (A)

    # E_fit_array = np.asarray(E_fit_plane(p_array, model.face_adjacency))
    E_dir_array = np.asarray(E_dir(r_array, face_area_array, model.face_adjacency))
    edges_array = model.faces_unique_edges.tolist()
    E_shape_array = np.asarray(E_shape(edges_array, edges_length, face_area_array,
                                       irregularity_array, model.face_adjacency))
    E_shape_array = E_shape_array.reshape((len(E_shape_array), 1))
    # E_sphere = E_fit_Sphere(model.face_adjacency, face_vertices )[:,None]
    # print('E_fit_plane = ' + str(np.hstack((model.face_adjacency.astype("object"), E_fit_array))))
    # print('E_dir = ' + str(np.hstack((model.face_adjacency.astype("object"), E_dir_array))))

    # print (e_fit)
    contraction_graph = nx.DiGraph()
    contraction_graph.add_nodes_from(range(0, len(model.faces)))
    counter = 0
    Normals = model.face_normals
    face_adjacency_info = get_face_adj_info(model, edges_array, edges_length, edges_vector_norm2, face_center)
    # betha = calculate_betha(model, edges_array, edges_length, edges_vector_norm2)
    n = len(model.faces)
    # E_fit_cylinders = np.asarray(E_fit_cylinder(model.face_adjacency, face_vertices, cov_list, face_adjacency_info, edges_array, face_area_array, face_center,
    #                contraction_graph, n))[:,None]
    # e_fit = np.minimum(E_fit_array, E_sphere)
    # e_fit = np.minimum(e_fit, E_fit_cylinders)
    # fit_primes = np.hstack((E_fit_array, E_sphere, E_fit_cylinders))
    # e_fit_primitive = fit_primes.min(axis=1)[:, None]
    E_fit_shapes_array = fit_shapes(model, model.face_adjacency, p_array, face_vertices, cov_list, face_adjacency_info, edges_array,
                        face_area_array, face_center, contraction_graph, n, voronoi_area_array, face_vertices_index)
    E_fit_array = E_fit(E_fit_shapes_array)
    # print(E_fit_array.shape, E_dir_array.shape, E_shape_array.shape)
    # dual_graph = nx.Graph()  # keep record of graph to guide later edge contraction
    dual_graph.add_weighted_edges_from(
        np.hstack((model.face_adjacency.astype("object"), E_fit_array + E_dir_array +  E_shape_array)))
    # plane_or_sphere_data = np.hstack((model.face_adjacency, np.where(E_fit_array < E_sphere, 0, 1)))
    # # print(plane_or_sphere_data)
    # map(lambda face: dual_graph.add_edge(face[0], face[1], plane_or_sphere=face[2]), plane_or_sphere_data)
    # print("test", np.argmin(fit_primes, axis=1))
    plane_or_sphere_data = np.hstack((model.face_adjacency, np.argmin(E_fit_shapes_array[:,0:3], axis=1)[:, None]))
    degree = np.hstack((model.face_adjacency, np.ones((model.face_adjacency.shape[0],1))))
    map(lambda face: dual_graph.add_edge(face[0], face[1], plane_or_sphere=face[2]), plane_or_sphere_data)
    node_info = [(u,d['degree']) for (u,d) in dual_graph.nodes(data = True)]
    # map(lambda face:dual_graph.add_edge(face[0], face[1], degree = max(node_info[face[0]][1], node_info[face[1]][1]) + 1), model.face_adjacency )
    map(lambda face: dual_graph.add_edge(face[0], face[1],degree=4),model.face_adjacency)

    A = len(dual_graph[0])
    print (A)
    B = dual_graph.neighbors(0)
    while (dual_graph.number_of_nodes() > 1):
        edges = np.array(map(lambda edge:[edge[0],edge[1],edge[2]['weight'],edge[2]['degree']],dual_graph.edges(data=True)))
        candidate_edges = edges[edges[:,2]==np.min(edges[:,2],axis=0)]
        candidate_edges = candidate_edges[candidate_edges[:,3]==np.min(candidate_edges[:,3])]

        # edge_to_contract = min(dual_graph.edges(data=True), key=lambda edge: edge[2]['weight'])  # find edge to contract, which connects face a to face b
        # min_weight = min([edge[2]['weight'] for edge in dual_graph.edges(data = True)])
        # print (min_weight)
        # candidate_edge = [(u,v,d) for (u,v,d) in dual_graph.edges(data=True) if d['weight'] == ]
        # candidate_edges =

        print (candidate_edges)

        # edge_to_contract = np.argmin([cand_edge[2]['degree'] for cand_edge in candidate_edge])
        edge_to_contract = candidate_edges[0]
        print (edge_to_contract)
        a = int(edge_to_contract[0])
        b = int(edge_to_contract[1])
        print ('a = ', a, 'b = ',b)
        if (a == 35):
            print ('check')

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

        # print(ex_edge)
        edges_array.append(ex_edge.tolist())
        new_face_vertices = np.vstack((face_vertices[a], face_vertices[b]))
        new_face_vertices = np.unique(new_face_vertices, axis=0)
        face_vertices.append(new_face_vertices)
        new_face_vertices_index = np.hstack((face_vertices_index[a], face_vertices_index[b]))
        face_vertices_index.append(np.unique(new_face_vertices_index))
        new_face_varea = voronoi_area_array[:,a]+ voronoi_area_array[:,b]
        voronoi_area_array = np.hstack((voronoi_area_array, new_face_varea [:,None]))
        face_prime = len(p_array) - 1  # all three arrays should be the same length, so it shouldn't matter which one we choose
        # dual_graph.add_node(int(face_prime), degree = max(dual_graph.node[a]['degree'], dual_graph.node[b]['degree'])+1)
        # dual_graph.add_node(int(face_prime))

        faces = []
        for neighbor in dual_graph.edges(a):  # find all edges where one of the verticies is a
            if (neighbor[1] != b):
                faces.append([face_prime, neighbor[
                    1]])  # don't need to check if a==b because we are preventing the creation of self loops when we contract
        for neighbor in dual_graph.edges(
                b):  # find all edges where one of the verticies is b and concat with previous list
            if (neighbor[1] != a):
                faces.append([face_prime, neighbor[1]])
        faces = np.array(faces).astype("int")

        contraction_graph.add_node(face_prime)
        contraction_graph.add_edge(face_prime, a)
        contraction_graph.add_edge(face_prime, b)
        all_decendants = list(nx.descendants(contraction_graph, face_prime))
        leaves = [i for i in all_decendants if i < len(model.faces)]
        # print (leaves)
        # c_cylinder, r, n = fit_cylinder(cov_prime, leaves, face_area_array, face_center, new_face_vertices)

        if len(faces) > 0:
            faces = np.unique(faces, axis=0)
            p = np.sum(edges_length[0][ex_edge])
            gamma = p ** 2 / (4 * np.pi * (face_area_array[a] + face_area_array[b]))
            irregularity_array.append(gamma)
            # e_fit_prime = np.asarray(E_fit_plane(p_array, faces))

            # print ('e_fit_prime = ' )
            # print (e_fit_prime)
            e_dir_prime = np.asarray(E_dir(r_array, face_area_array, faces))
            if (faces.shape[0] > 1):
                e_shape_prime = np.asarray(E_shape(edges_array, edges_length, face_area_array,
                                                   irregularity_array, faces))
                e_shape_prime = e_shape_prime.reshape((len(e_shape_prime), 1))
            else:
                e_shape_prime = np.array([[0]])
            # e_fit_sphere_prime = E_fit_Sphere(faces, face_vertices)[:, None]
            # e_fit_cylinders = np.asarray(
            #     E_fit_cylinder(faces, face_vertices, cov_list, face_adjacency_info, edges_array, face_area_array,
            #                    face_center, contraction_graph, n))[:, None]
            # print('e_fit_sphere_prime = ')
            # print(e_fit_sphere_prime)
            e_fit_shapes_prime = fit_shapes(model, faces,p_array,face_vertices,cov_list,face_adjacency_info,edges_array,face_area_array,face_center,contraction_graph,n, voronoi_area_array, face_vertices_index)
            e_fit_prime = E_fit(e_fit_shapes_prime)
            # fit_prime = np.hstack((e_fit_prime, e_fit_sphere_prime, e_fit_cylinders))
            E_fit_shapes_array = np.vstack((E_fit_shapes_array, e_fit_shapes_prime))  # add our new fit prime to the larger fit list
            # e_fit_primitive = fit_prime.min(axis=1)[:, None]
            # for i in range(e_fit_primitive.shape[0]):
            #     if e_fit_primitive[i,0] == e_fit_sphere_prime[i,0]:
            #         print ('sphere ')
            #     else:
            #         print ('plane')

        dual_graph.add_node(int(face_prime))

        degree_weighted = (e_fit_prime + e_dir_prime + e_shape_prime)
        dual_graph.add_weighted_edges_from(np.hstack((faces, degree_weighted)))  # reconnect the new node
        plane_or_sphere_data = np.hstack((faces, np.argmin(e_fit_shapes_prime[:,0:3], axis=1)[:, None]))
        map(lambda face: dual_graph.add_edge(face[0], face[1], plane_or_sphere = face[2]), plane_or_sphere_data)
            # test = map(lambda face:[face[0], face[1], [dual_graph[face[0]] , dual_graph[face[1]]]], faces )
        colors = [[229, 156, 11, 255], [11, 204, 229, 255], [226, 30, 9, 255]]
        model.visual.face_colors[leaves] = trimesh.visual.random_color()
        print("color", dual_graph.get_edge_data(a, b)["plane_or_sphere"])
        model.visual.face_colors[leaves] = colors[dual_graph.get_edge_data(a, b)["plane_or_sphere"]]
        dual_graph.remove_node(a)  # remove the old nodes
        dual_graph.remove_node(b)

        degree = map(lambda face: len(dual_graph[face[0]]) + len (dual_graph[face[1]]) -2, faces )
        edge_degree = np.hstack((faces, np.asarray(degree)[:,None]))
        map(lambda face: dual_graph.add_edge(face[0], face[1], degree=face[2]), edge_degree)


        # colors = [[229, 156, 11, 255], [11, 204, 229, 255], [226, 30, 9, 255]]  # plane, sphere, cylinder

        # model.visual.face_colors[leaves] = trimesh.visual.random_color()
        # print("color", dual_graph.get_edge_data(a, b)["plane_or_sphere"])
        # model.visual.face_colors[leaves] = colors[dual_graph.get_edge_data(a, b)["plane_or_sphere"]]
        # if (dual_graph.get_edge_data(a,b)["plane_or_sphere"]==0):
        #     model.visual.face_colors[leaves] = color_plane
        #     # model.visual.face_colors[leaves] = trimesh.visual.random_color()
        # else:
        #     model.visual.face_colors[leaves] = color_sphere
        #     # model.visual.face_colors[leaves] = trimesh.visual.random_color()


        # contraction_graph.add_node(face_prime)
        # contraction_graph.add_edge(face_prime,a)
        # contraction_graph.add_edge(face_prime,b)

        # all_decendants = list(nx.descendants(contraction_graph, face_prime))
        # leaves = [i for i in all_decendants if i < len(model.faces)]
        # print(leaves)

        # print(a , b)
        # if (dual_graph.number_of_nodes() == 2):
        # if(counter == 166):
        if (counter %1  == 0):
            print(e_fit_shapes_prime)
            model.show(smooth=False)
        print(counter)
        counter = counter + 1

    pos = hierarchy_pos(contraction_graph, len(p_array) - 1)
    nx.draw(contraction_graph, pos=pos, with_labels=True)

    plt.show()
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


def basic_contraction_graph_to_seg(contraction_graph, model):
    root = len(contraction_graph) - 1  # makes the assumtion that the last node added is the root
    clusters = []
    for node in contraction_graph[root].keys():
        for node2 in contraction_graph[node].keys():
            clusters.append(node2)
    print(list(clusters))
    print("test")
    seg = np.zeros((len(model.faces), 1)).astype("int")
    for i, cluster in enumerate(clusters):
        print(i)
        leaves = [j for j in list(nx.descendants(contraction_graph, cluster)) if j < len(model.faces)]
        seg[leaves] = i
    return seg
