#ifndef BINARY_TO_LEVEL_SET_H
#define BINARY_TO_LEVEL_SET_H
#include <string>
#include <iostream>

#include <openvdb/openvdb.h>
#include <openvdb/tools/TopologyToLevelSet.h>

#include "Reorienter.h"


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
     * of on/off values. This also reorients the grid
     */
    void populate_grid(const BinvoxData& data);
    /**
     * Convert the binary voxel grid -> level set using OpenVDB's built-in
     * methods
     *
     * half_bandwidth: half of the narrow band width in voxels. 3 is a good
     *    number for the source model in a morph, but set it higher for the
     *    target model if there are aliasing issues
     * smoothing_steps: how many steps of smoothing. 5 is a good upper bound. 
     */
    void convert(float half_bandwidth, int smoothing_steps);
    /**
     * Save the resulting level set 
     *
     * This method can only be called after convert()
     */
    void save(std::string filename);
private:
    /**
     * Usinge the data from the binvox data, keep track of the bounding
     * box and centroid and use the Reorienter class (see Reorienter.h/.cpp)
     * to determine an affine transformation that standardizes the model
     * orientation.
     */
    openvdb::math::Transform::Ptr compute_transform(const BinvoxData& data);
    /**
     * Handle a single run in the run-length encoded voxel data. This applies
     * the reorientation transformation to each voxel to fix an issue
     * with OpenVDB being unable to morph general affine transformations
     */
    void populate_run(
        const openvdb::math::Transform::Ptr& reorient_xform, 
        int length, 
        int start_voxel);
    /**
     * Add a run of voxels to a reorienter
     */
    void update_reorienter(
        Reorienter& reorienter, int length, int start_voxel);

    /**
     * Convert a 1D index to (x, y, z coordinates)
     */
    openvdb::Coord index_to_coord(int index);
    
};

#endif
