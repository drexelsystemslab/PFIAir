#ifndef LEVEL_SET_WRAPPER_H
#define LEVEL_SET_WRAPPER_H
#include <string>
#include <memory>
#include <openvdb/openvdb.h>
#include "Mesh.h"

class Mesh;

/**
 * Utility class that helps iterate over surface values (read-only)
 * I stray away from the C++ standard iterators since they are
 * more complicated than they are worth
 *
 * Typical Usage: 
 *
 * LevelSet level_set(...); 
 * SurfaceIterator surf_iter = level_set.get_surface_iterator();
 * for (surf_iter.begin(); surf_iter; surf_iter++) {
 *    // Do whatever with surf_iter.get_value()
 *    // and surf_iter.get_coord()
 * }
 */
class SurfaceIterator {
    // The OpenVDB grid to iterate over
    openvdb::FloatGrid::ConstPtr grid; 

    // an iterator object which points to the current active voxel
    openvdb::FloatGrid::ValueOnCIter iter;
public:
    // Null constructor. It's a bit hacky since there's no way to
    // null-construct OpenVDB iterators
    SurfaceIterator(): grid(nullptr), iter(openvdb::FloatTree()) {}

    // This is the constructor to use. See usage notes above
    SurfaceIterator(openvdb::FloatGrid::ConstPtr grid): 
        grid(grid),
        // This is redundant since SurfaceIterator::begin() does the
        // same thing. However, there is no null constructor
        // for OpenVDB iterators
        iter(grid->cbeginValueOn()) {}

    // Start iterating over the underlying grid
    void begin();
    
    // Return true if the iterator is not finished
    operator bool();

    // Get the underlying OpenVDB iterator object to access the
    // value/coordinate
    openvdb::FloatGrid::ValueOnCIter vdb_iter();

    // Get the coordinate in 3D space for the current
    // location. This is shorthand for vdb_iter().getCoord()
    openvdb::Coord get_coord();

    // Get the value at the current location
    // shortcut for vdb_iter().getValue()
    double get_value();
    
    /**
     * Advance the underlying iterator
     */
    void operator++(int);

    /**
     * Check if this voxel is on the 0 surface. This is done
     * by checking if we are a negative voxel adjacent to a positive
     * voxel
     */
    bool is_surface_voxel();
};

/**
 * The actual LevelSet, a higher-level wrapper around an
 * OpenVDB level set (a openvdb::FloatGrid::Ptr with a grid class of 
 * openvdb::GRID_LEVEL_SET)
 */
class LevelSet {
    // Level set representation as a VDB data structure
    openvdb::FloatGrid::Ptr level_set = nullptr;
public:
    static constexpr double HALF_BANDWIDTH = 3.0;
    static constexpr double VOXEL_SIZE = 0.01;
    static constexpr int NORM_COUNT = 10;

    // How many voxels to offset when doing the morphological opening
    static constexpr int OPENING_VOXELS = 5;

    /**
     * Null constructor for declarations
     */
    LevelSet() {};

    /**
     * Constructor from a .vdb file. Only use .vdb filees that have a single
     * model in them
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
        bool is_open_mesh,
        double half_bandwidth=HALF_BANDWIDTH);

    /**
     * Initialize from a grid pointer.
     */
    LevelSet(openvdb::FloatGrid::Ptr level_set): level_set(level_set) {}

    // Getter for the underlying VDB
    openvdb::FloatGrid::Ptr get_level_set();
    openvdb::FloatGrid::ConstPtr get_level_set() const;

    /**
     * Perform a morphological opening
     */
    void morphological_opening();

    /**
     * Count the number of surface voxels in the model
     */
    int count_surface_voxels() const;

    /**
     * Get the voxel size of the underlying level set
     */
    double get_voxel_size() const;

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
     * Deep copy the underlying level set and 
     * return a new LevelSet object.
     */
    LevelSet deep_copy() const;

    /**
     * Save a .vdb file of this level set
     */
    void save(std::string fname) const;

    /**
     * Get a SurfaceIterator object which can be used to iterate over
     * surface voxels only
     */
    SurfaceIterator get_surface_iterator() const;
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
