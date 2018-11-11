#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H
#include <vector>
#include <string>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <openvdb/openvdb.h>

#include "MostCentralVertex.h"
#include "ClosestToCentroid.h"
#include "BoundingBox.h"

/**
 * This new Mesh class consolidates the code
 * from Container and UpdtMeshOperations into
 * an easier to use class
 */
class Mesh {
    // Index of the w-component of a vector
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 3;

    // Homogeneous coordinates require size 4 vectors
    static const int VECTOR_SIZE = 4;

    /**
     * List of vertices and faces from the .obj
     * file stored as OpenVDB vectors.
     */
    std::vector<openvdb::Vec3s> vertices;
    std::vector<openvdb::Vec3I> indices_tri;
    std::vector<openvdb::Vec4I> indices_quad;

    /**
     * Geometry matrix. This stores the vertices in homogeneous
     * coordinates for matrix multiplication.
     */
    Eigen::MatrixXd geometry;

    /**
     * The geometric centroid of the current mesh vertices.
     * This gets transformed whenever the mesh is transformed.
     */
    Eigen::Vector4d centroid;
    
    /**
     * The "most central vertex". There are different ways to compute this
     * either via eccentricity (accurate, but very slow since it uses 
     * Floyd-Warshall in O(V^3) time), or closest vertex to centroid (less
     * accurate, but runs in O(V) time.
     */
    Eigen::Vector4d central_vertex;

    /**
     * The current bounding box for this model.
     */
    BoundingBox bbox; 

    /**
     * The mesh has extra properties that are not always calculated.
     * These flags determine what fields are valid.
     */
    bool vertices_valid = false;
    bool matrix_valid = false;
    bool centroid_valid = false;
    bool central_vertex_valid = false;
    bool bbox_valid = false;

    /**
     * Marks this mesh as open or closed. This makes
     * a difference when we convert to a VDB.
     */
    bool is_open_mesh = false;
public: 
    // Constructors =============================================
    /**
     * Null constructor
     */
    Mesh() {};
    /**
     * Constructor from a .obj file.
     * Right now, the caller must specify whether the mesh is open or
     * closed.
     *
     * This populates the three lists and sets vertices_valid to true
     */
    Mesh(std::string filename, bool is_open_mesh);

    /**
     * Construct a mesh from lists of vertices and faces.
     * This is used, for example, when converting a VDB -> Mesh
     */
    Mesh(
            std::vector<openvdb::Vec3s> vertex_list,
            std::vector<openvdb::Vec3I> triangle_list,
            std::vector<openvdb::Vec4I> quad_list,
            bool is_open_mesh);

    /**
     * Construct a mesh from an Eigen matrix.
     * TODO: Is this needed?
     */
    Mesh(Eigen::MatrixXd& vertices, bool is_open_mesh);

    /**
     * Preprocess the mesh. This calls either preprocess_open_mesh()
     * or preprocess_cclosed_mesh() depending on the flag set at instantiation.
     */
    void preprocess_mesh();

    /**
     * Save the OBJ file to disk. This is useful for debugging.
     */
    void save_obj(std::string filename);
private:

    /**
     * Preprocess an open mesh.
     * This is a slightly different sequence of events from preprocessing
     * closed meshes.
     */
    void preprocess_open_mesh();
    
    /**
     * Preprocess a closed mesh.
     */
    void preprocess_closed_mesh();

    // Populate optional fields ======================================

    /**
     * Populate the matrix representation of this object
     */
    void calc_matrix();

    /**
     * After transforming the matrix, the vertex list is stale.
     * convert the matrix back into a list of vertices.
     */
    void calc_vertex_list();

    /**
     * Compute the centroid of this mesh's vertices.
     */
    void calc_centroid();

    /**
     * Compute the central vertex by finding the vertex with lowest
     * eccentricity. This is more accurate, but slow, taking O(V^3) time.
     */
    void calc_central_vertex(MostCentralVertex& center_finder);

    /**
     * Compute the most central vertex by finding the closest vertex
     * to the centroid. This takes O(V) time.
     */
    void calc_central_vertex(ClosestToCentroid& center_finder);

    /**
     * Calculate the bounding box for this mesh.
     */
    void calc_bounding_box();

    // Transform Mesh =============================================

    /**
     * Apply a transformation to the matrix representation and
     * any vectors computed so far. The vertex list is invalidated
     */
    void apply_transform(const Eigen::Matrix4d& xform);

    /**
     * Translate this mesh so its geometric centroid is now the origin
     */
    void center_on_centroid();

    /**
     * If the max length of this mesh is < 1, scale the mesh so its
     * max length is now 1.
     * This should be used BEFORE PCA
     */
    void scale_up_small_mesh();

    /**
     * Convert to a VDB, perform a morphological opening, and scan convert
     * back to a mesh. 
     */
    void resample();

    /**
     * Perform Principal Component Analysis to find the major axis
     * of the model. Then rotate the mesh so the axis is to the right.
     */
    void perform_pca();

    /**
     * Scale the mesh so tha max length is now 1
     * this should be used AFTER PCA
     */
    void normalize_scale();

    /**
     * Though the model is axis-aligned after PCA, it might be flipped
     * 180 degrees. Rotate the model so it has a non-positive skewness.
     */
    void normalize_skewness();

    /**
     * Translate the mesh so it is now centered on its central vertex
     * This is only done for
     */
    void center_on_central_vertex();

    // Methods for parsing an OBJ file ================================
    /**
     * Entry point for reading an OBJ file. This populates
     * all three lists of vectors
     */
    void parse_obj_file(std::string filename);
    /**
     * Handle OBJ lines of the form
     * v x y z ...
     * populating the vertex list
     */
    void parse_obj_vertex(std::vector<std::string> tokens);
    /**
     * Handle OBJ lines of the form
     * f a b c
     * f a b c d
     * where a, b, c, d are face indices
     * as specified here:
     *
     * https://en.wikipedia.org/wiki/Wavefront_.obj_file#Face_elements
     * 
     * Note: Any texture coordinates or normals are discarded
     * for this method.
     */
    void parse_obj_face(std::vector<std::string> tokens);

    // Methods for saving an OBJ file ==================================

    /**
     * Format the vertices and faces to OBJ format
     * for writing to disk. Each of these methods writes
     * a single line to the file
     */
    void write_vertex(std::ostream& stream, openvdb::Vec3s vertex);
    void write_triangle(std::ostream& stream, openvdb::Vec3I triangle);
    void write_quad(std::ostream& stream, openvdb::Vec4I quad);
};

#endif
