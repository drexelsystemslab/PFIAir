#ifndef LEVEL_SET_WRAPPER_H
#define LEVEL_SET_WRAPPER_H
#include <string>
#include <openvdb/openvdb.h>
#include "Mesh.h"

class Mesh;

class LevelSet {
    // Level set representation as a VDB data structure
    openvdb::FloatGrid::Ptr level_set = nullptr;
    /**
     * Short name to identify this level set.
     */
    std::string name;
public:
    /**
     * Half bandwidth of the final signed distance field.
     */
    static constexpr float HALF_BANDWIDTH = 3.0;
    static constexpr double VOXEL_SIZE = 0.01;
    static constexpr int NORM_COUNT = 10;

    // How many voxels to offset when doing the morphological opening
    static constexpr int OPENING_VOXELS = 5;

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
     * Count the number of surface voxels in the model
     */
    int count_surface_voxels();

    /**
     * Check if a voxel is on the surface of the level set
     */
    bool is_surface_voxel(const openvdb::Coord& coord);

    /**
     * Convert a LevelSet to a new Mesh object
     */
    Mesh to_mesh();

    // Getters and setters
    std::string get_name() const;
    void set_name(std::string name);

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
     * 0 point and subtracting across all space. Note that the
     * unsigned field must have TWICE the desired bandwidth
     *
     *
     * Example:
     * Before: unsigned distance field looks like this (measured in voxels)
     *
     * Note: Every isoband except for the one for 0 is a closed surface
     *
     *  ------------------ 6
     *  ------------------ 5
     *  ------------------ 4
     *  ------------------ 3  <-- 3 will be the new 0 point
     *  ------------------ 2
     *  ------------------ 1
     *  ================== 0 <-- original surface
     *  ------------------ 1
     *  ------------------ 2
     *  ------------------ 3  <-- 3 will be the new 0 point
     *  ------------------ 4
     *  ------------------ 5
     *  ------------------ 6
     *
     * After: subtract current distance from new zero point:
     *
     * Note: again, each isoband except for -3 is a closed surface. So
     * our new 0 point is a closed surface
     *
     *  ------------------ 3 <-- outside the surface is positive
     *  ------------------ 2
     *  ------------------ 1
     *  ================== 0  <-- new surface
     *  ------------------ -1
     *  ------------------ -2
     *  ------------------ -3 <-- Inside the surface is negative
     *  ------------------ -2
     *  ------------------ -1
     *  ================== 0  <-- The same new surface
     *  ------------------ 1
     *  ------------------ 2
     *  ------------------ 3 <-- outside the surface  is positive
     *
     */
    void convert_unsigned_to_signed();
};

#endif
