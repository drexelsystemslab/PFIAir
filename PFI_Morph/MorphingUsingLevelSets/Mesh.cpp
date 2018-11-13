#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <eigen3/Eigen/Geometry>
#include "PCACalculator.h"

const int Mesh::X;
const int Mesh::Y;
const int Mesh::Z;
const int Mesh::W;
const int Mesh::VECTOR_SIZE;

Mesh::Mesh(std::string filename, bool is_open_mesh):
        is_open_mesh(is_open_mesh) {

    // Read the vertices and mark themm as valid
    parse_obj_file(filename);
    vertices_valid = true; 
}

void Mesh::preprocess_mesh() {
    // Select a path depending on whether the mesh is open or closed.
    if (is_open_mesh)
        preprocess_open_mesh();
    else
        preprocess_closed_mesh();
}

void Mesh::save_obj(std::string filename) {
    std::ofstream obj_file(filename);

    if (!obj_file.is_open())
        throw std::runtime_error("Could not open output file " + filename);

    // Write the vertices...
    obj_file << "# Vertices" << std::endl;
    for (const openvdb::Vec3s& vertex : vertices)
        write_vertex(obj_file, vertex);

    // ...the triangle face list...
    obj_file << "# Triangles" << std::endl;
    for (const openvdb::Vec3I& tri : indices_tri)
        write_triangle(obj_file, tri);

    // ...and the quad face list
    obj_file << "# Quads" << std::endl;
    for (const openvdb::Vec4I& quad : indices_quad)
        write_quad(obj_file, quad);

    obj_file.close();
}

void Mesh::preprocess_open_mesh() {
    // Most operations will need the matrix representation of this mesh.
    calc_matrix();

    // Center the mesh on its centroid. This is needed before any scaling
    // operations
    calc_centroid();
    center_on_centroid();

    // Small meshes need to be scaled up.
    calc_bounding_box();
    scale_up_small_mesh();

    // Find the "most central vertex" one way or another
    ClosestToCentroid center_finder;
    //MostCentralVertex center_finder;
    calc_central_vertex(center_finder);

    // convert to a VDB and perform a morphological opening to
    // smooth out the mesh.
    resample();

    // re-center the mesh before performing PCA
    calc_centroid();
    center_on_centroid();

    // Perform PCA and orient the mesh so it is axis-aligned.
    perform_pca();

    // Scale the axis-aligned mesh so the max length is 1
    calc_bounding_box();
    normalize_scale();

    // Normalize meshes so the skewness is always non-positive
    normalize_skewness();

    // Recenter the mesh on the most central vertex.
    // NOTE: the transformations above modify the most central
    // vertex, see transform() for more information.
    center_on_central_vertex();
}

void Mesh::preprocess_closed_mesh() {
    std::cout << "TODO: Fill out preprocess_closed_mesh" << std::endl;
}

void Mesh::calc_matrix() {
    if (!vertices_valid)
        throw std::runtime_error("Cannot build matrix from invalid vertices!");

    // Initialize the matrix.
    geometry = Eigen::MatrixXd(VECTOR_SIZE, vertices.size()); 
    for (unsigned int i = 0; i < vertices.size(); i++) {
        // Store the vector in homogeneous coordinates.
        openvdb::Vec3s vertex = vertices[i];
        geometry(X, i) = vertex.x();
        geometry(Y, i) = vertex.y();
        geometry(Z, i) = vertex.z();
        geometry(W, i) = 1.0;
    }

    // Mark the matrix field as valid
    matrix_valid = true;
}

void Mesh::calc_centroid() {
    if (!matrix_valid)
        throw std::runtime_error("Matrix must be valid to compute centroid");

    int N = geometry.cols();
    centroid = geometry.rowwise().sum() / N;

    // Sums of homogeneous coordinates should not modify the w-component.
    centroid(W) = 1.0;

    // Mark the centroid as valid.
    centroid_valid = true;
}

void Mesh::apply_transform(const Eigen::Matrix4d& xform) {
    // Apply the transformation to the geometry matrix
    geometry = xform * geometry;

    // Also apply it to the centroid and/or central vertex if they exist
    if (centroid_valid) {
        // Apply the transformation and re-normalize the homogeneous coords
        centroid = xform * centroid;
        centroid /= centroid(W);
    }

    if (central_vertex_valid) {
        // Apply the transformation and re-normalize the homogeneous coords
        central_vertex = xform * central_vertex;
        central_vertex /= central_vertex(W);
    }

    // The vertices and bounding box are no longer valid
    vertices_valid = false;
    bbox_valid = false;
}

void Mesh::center_on_centroid() {
    if (!centroid_valid)
        throw std::runtime_error("Cannot center model on invalid centroid");

    Eigen::Vector3d delta = -centroid.head(VECTOR_SIZE - 1);
    Eigen::Translation3d translate(delta);
    Eigen::Affine3d xform(translate);

    // Apply the transformation to the matrix
    apply_transform(xform.matrix());
}

void Mesh::calc_bounding_box() {
    if (!matrix_valid)
        throw std::runtime_error(
            "Matrix must be valid to compute bounding box");

    // Scan across the matrix and compute maximmum and minimum
    // coordinates on each axis.
    Eigen::Vector4d max_coords = geometry.rowwise().maxCoeff();
    Eigen::Vector4d min_coords = geometry.rowwise().minCoeff();

    // Build a bounding box
    // from (x, y, z) ignoring w
    bbox = BoundingBox(
        min_coords.head(VECTOR_SIZE - 1), 
        max_coords.head(VECTOR_SIZE - 1));
    bbox_valid = true;
}

void Mesh::scale_up_small_mesh() {
    if (!bbox_valid)
        throw std::runtime_error("Cannot scale meshes without bounding box!");

    // If the max length is less than 1, scale up the mesh to 1
    double max_length = bbox.get_max_length();
    if (max_length < 1.0) {
        Eigen::UniformScaling<double> inv_scale = 
            Eigen::Scaling(1.0 / max_length);
        Eigen::Affine3d xform(inv_scale);
        apply_transform(xform.matrix());
    }
}

void Mesh::calc_central_vertex(ClosestToCentroid& center_finder) {
    std::cout << "TODO: Fill out calc_central_vertex" << std::endl;
}

void Mesh::resample() {
    std::cout << "TODO: Fill out resample" << std::endl;
}

void Mesh::perform_pca() {
    if (!matrix_valid)
        throw std::runtime_error("Matrix must be valid to perform PCA");

    PCACalculator calc;
    Eigen::Matrix4d rotation = calc.perform_pca(geometry);
}

void Mesh::normalize_scale() {
    std::cout << "TODO: Fill out normalize_scale" << std::endl;
}

void Mesh::normalize_skewness() {
    std::cout << "TODO: Fill out normalize_skewness" << std::endl;
}

void Mesh::center_on_central_vertex() {
    if (!central_vertex_valid)
        throw std::runtime_error("Cannot center model on invalid centroid");

    Eigen::Vector3d delta = -central_vertex.head(VECTOR_SIZE - 1);
    Eigen::Translation3d translate(delta);
    Eigen::Affine3d xform(translate);

    // Apply the transformation to the matrix
    apply_transform(xform.matrix());
}

void Mesh::parse_obj_file(std::string filename) {
    // Open the file
    std::ifstream obj_file(filename);
    if (!obj_file.is_open())
        throw std::runtime_error("Cannot open OBJ file " + filename);

    // Iterate over the lines
    std::string line;
    while(std::getline(obj_file, line)) {
        // Divide the lines into tokens
        std::istringstream iss(line);
        std::vector<std::string> tokens{
            std::istream_iterator<std::string>{iss}, 
            std::istream_iterator<std::string>{}};

        // skip empty lines
        if (tokens.size() == 0)
            continue;

        // Look at the first token to see which type of line this is
        if (tokens[0] == "v") {
            // Handle vertex coordinates
            parse_obj_vertex(tokens);
        } else if (tokens[0] == "f") {
            // Handle face indices
            parse_obj_face(tokens);
        } else {
            // Skip all other lines quietly
        }
        
    }

    obj_file.close();
}

void Mesh::parse_obj_vertex(std::vector<std::string> tokens) {
    // Make sure we have at least f x y z [...]
    if (tokens.size() < 4)
        throw std::runtime_error(
            "Too few components for vertex: " + std::to_string(tokens.size()));

    // Unpack a vector
    float x = std::stof(tokens[1]);
    float y = std::stof(tokens[2]);
    float z = std::stof(tokens[3]);
    openvdb::Vec3s vector(x, y, z);

    // Add it to the vertex list
    vertices.push_back(vector); 
}

void Mesh::parse_obj_face(std::vector<std::string> tokens) {
    // Triangles are of the form f v1 v2 v3
    if (tokens.size() < 4)
        throw std::runtime_error(
            "Too few indices for triangle: " + std::to_string(tokens.size()));

    // Quads are of the form f v1 v2 v3 v4
    if (tokens.size() > 5)
        throw std::runtime_error(
            "Too many indices for quad: " + std::to_string(tokens.size()));

    /**
     * Implementation detail:
     * OBJ face component lines are of the form
     * f vertex1/texture1/normal1 vertex2/texture2/normal2 ...
     * where textures and normals are optional
     * For this application, we only need the vertices, so 
     * a simple call to std::stoi will get everything up to the first slash.
     *
     * if normals are ever needed, change how this function is implemented
     */
    
    if (tokens.size() == 4) {
        // Parse a triangle's vertex indices
        // NOTE: .obj files use 1-based indexing for vertices. Subtract 1
        // since we want 0-based indices
        int v1 = std::stoi(tokens[1]) - 1;
        int v2 = std::stoi(tokens[2]) - 1;
        int v3 = std::stoi(tokens[3]) - 1;
        openvdb::Vec3I triangle(v1, v2, v3);

        indices_tri.push_back(triangle);
    } else {
        // Same here, but parse a quad instead
        int v1 = std::stoi(tokens[1]) - 1;
        int v2 = std::stoi(tokens[2]) - 1;
        int v3 = std::stoi(tokens[3]) - 1;
        int v4 = std::stoi(tokens[4]) - 1;
        openvdb::Vec4I quad(v1, v2, v3, v4);

        indices_quad.push_back(quad);
    }
}

void Mesh::write_vertex(std::ostream& stream, openvdb::Vec3s vertex) {
    // Format: v x y z
    stream << "v " << vertex.x() 
        << " " << vertex.y() 
        << " " << vertex.z() 
        << std::endl;
}

void Mesh::write_triangle(std::ostream& stream, openvdb::Vec3I triangle) {
    // Format: f v1 v2 v3
    // Note that these are 3 indices, not true coordinates
    // Also note that the + 1 is to make the indices 1-based instead of
    // 0-based
    stream << "f " << triangle.x() + 1
        << " " << triangle.y() + 1
        << " " << triangle.z() + 1
        << std::endl;
}

void Mesh::write_quad(std::ostream& stream, openvdb::Vec4I quad) {
    // Format: f v1 v2 v3 v4
    // Note that these are 4 indices, not true coordinates
    // Also note that the + 1 is to make the indices 1-based instead of
    // 0-based
    stream << "f " << quad.x() + 1
        << " " << quad.y() + 1
        << " " << quad.z() + 1
        << " " << quad.w() + 1
        << std::endl;
}
