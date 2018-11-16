#ifndef LEVEL_SET_WRAPPER_H
#define LEVEL_SET_WRAPPER_H
#include <string>
#include <openvdb/openvdb.h>
#include "Mesh.h"

class Mesh;

class LevelSet {
    // Level set representation as a VDB data structure
    openvdb::FloatGrid::Ptr level_set = nullptr;
public:
    /**
     * Half bandwidth of the final signed distance field.
     */
    static constexpr float HALF_BANDWIDTH = 3.0;
    static constexpr double VOXEL_SIZE = 0.01;

    /**
     * Constructor from a .vdb file 
     */
    LevelSet(std::string fname);
    /**
     * Constructor from vertex list
     * This is mainly used by Mesh::to_level_set();
     */
    LevelSet(
        const std::vector<openvdb::Vec3s>& vertices,
        const std::vector<openvdb::Vec3I>& indices_tri,
        const std::vector<openvdb::Vec4I>& indices_quad,
        bool is_open_mesh);

    // Getter for the underlying VDB
    openvdb::FloatGrid::Ptr get_level_set();

    /**
     * Perform a morphological opening
     */
    void morphological_opening();

    /**
     * Convert a LevelSet to a new Mesh object
     */
    Mesh to_mesh();

    /**
     * Convert to a mesh, populating lists of vertices and faces.
     * This is used to update an existing Mesh
     */
    void to_mesh(
        std::vector<openvdb::Vec3s>& out_vertices,
        std::vector<openvdb::Vec3I>& out_indices_tri,
        std::vector<openvdb::Vec4I>& out_indices_quad);

    /**
     * Save a .vdb file of this level set
     */
    void save(std::string fname);
private:
    /**
     * Convert an unsigned distance field to a signed by setting a new
     * 0 point and subtracting across all space
     */
    void convert_unsigned_to_signed(float bandwidth);
};

#endif
