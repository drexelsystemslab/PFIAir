#include "Mesh.h"

Mesh::Mesh(std::string filename, bool is_open_mesh):
        is_open_mesh(is_open_mesh) {
    std::cout << "TODO: Fill out Constructor(string, bool)" << std::endl;
}

void Mesh::preprocess_mesh() {
    // Select a path depending on whether the mesh is open or closed.
    if (is_open_mesh)
        preprocess_open_mesh();
    else
        preprocess_closed_mesh();
}

void Mesh::save_obj(std::string filename) {
    std::cout << "TODO: Fill out save_obj" << std::endl;
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
    std::cout << "TODO: Fill out calc_matrix" << std::endl;
}

void Mesh::calc_centroid() {
    std::cout << "TODO: Fill out calc_centroid" << std::endl;
}

void Mesh::center_on_centroid() {
    std::cout << "TODO: Fill out center_on_centroid" << std::endl;
}

void Mesh::calc_bounding_box() {
    std::cout << "TODO: Fill out bounding_box" << std::endl;
}

void Mesh::scale_up_small_mesh() {
    std::cout << "TODO: Fill out scale_up_small_mesh" << std::endl;
}

void Mesh::calc_central_vertex(ClosestToCentroid& center_finder) {
    std::cout << "TODO: Fill out calc_central_vertex" << std::endl;
}

void Mesh::resample() {
    std::cout << "TODO: Fill out resample" << std::endl;
}

void Mesh::perform_pca() {
    std::cout << "TODO: Fill out perform_pca" << std::endl;
}

void Mesh::normalize_scale() {
    std::cout << "TODO: Fill out normalize_scale" << std::endl;
}

void Mesh::normalize_skewness() {
    std::cout << "TODO: Fill out normalize_skewness" << std::endl;
}

void Mesh::center_on_central_vertex() {
    std::cout << "TODO: Fill out center_on_central_vertex" << std::endl;
}
