#ifndef BINARY_TO_LEVEL_SET_H
#define BINARY_TO_LEVEL_SET_H
#include <string>
#include <iostream>

#include <openvdb/openvdb.h>
#include <openvdb/tools/TopologyToLevelSet.h>

// Binvox data is stored as pairs of (value, run length)
typedef std::pair<int, int> RunLength;
typedef std::vector<RunLength> BinvoxData;

class BinaryToLevelSet {
    // Transformation parameters from index -> world space
    float scale;
    openvdb::Vec3d translation;
    // Dimensions in index space
    openvdb::Vec3I dimensions;
    // This grid contains a 1 whever there is a voxel. 0 voxels are not
    // stored.
    openvdb::FloatGrid::Ptr binary_grid = nullptr;
    // The converted level set after calling convert()
    openvdb::FloatGrid::Ptr level_set = nullptr;
public:
    /**
     * Set the dimensions of the grid. Normally d = w = h, but the code
     * does not make that assumption just in case.
     */
    void set_dimensions(int d, int w, int h);
    /**
     * Set the origin of the model
     */
    void set_translation(float x, float y, float z);
    /**
     * Set the model's scale in world space
     */
    void set_scale(float scale);
    /**
     * Populate the grid from a sequence of run-length encoded pairs
     * of on/off values
     */
    void populate_grid(const BinvoxData& data);
    /**
     * Convert the binary voxel grid -> level set using OpenVDB's built-in
     * methods
     */
    void convert();
    /**
     * Save the resulting level set 
     *
     * This method can only be called after convert()
     */
    void save(std::string filename);
private:
    /**
     * Handle a single run in the run-length encoded voxel data
     */
    void populate_run(int length, int start_voxel);

    /**
     * Convert a 1D index to (x, y, z coordinates)
     */
    openvdb::Coord index_to_coord(int index);
    
};

#endif
