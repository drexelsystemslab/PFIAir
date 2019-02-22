#include "BinaryToLevelSet.h"

void BinaryToLevelSet::set_dimensions(int d, int w, int h) {
    dimensions = openvdb::Vec3I(d, w, h);
}

void BinaryToLevelSet::set_translation(float x, float y, float z) {
    translation = openvdb::Vec3d(x, y, z);
}

void BinaryToLevelSet::set_scale(float scale) {
    this->scale = scale; 
}

void BinaryToLevelSet::populate_grid(const BinvoxData& data) {
    binary_grid = openvdb::FloatGrid::create(0.0);

    int voxel_count = 0;

    /**
     * Utilitty for computing the correct translation/rotation of the model
     * that uses the bounding box and centroid to reorient the model
     */
    Reorienter reorienter;

    // Loop over the runs of voxels in the file and populate the grid
    for (const RunLength& run : data) {
        // Value is either 0 to 1
        int value = run.first;
        int length = run.second;

        if (value == 0) {
            // Don't store 0 values in the level set. This is necessary
            // for topologyToLevelSet() to work properly
            voxel_count += length;
        } else {
            // Update the VDB and the reorienter's centroid/bounding box
            populate_run(length, voxel_count);
            update_reorienter(reorienter, length, voxel_count);
            voxel_count += length;
        }
    }

    // Set the scale and origin =============================
    using namespace openvdb::math;

    // Use the transformation given by the reorienter object
    // since the info given in the binvox file is not standardized in the
    // way we need.
    Transform::Ptr xform = reorienter.compute_transform();
    binary_grid->setTransform(xform);
}

/**
 * Write a sequence of 1s into the grid starting at start_voxel
 */
void BinaryToLevelSet::populate_run(int length, int start_voxel) {
    openvdb::FloatGrid::Accessor acc = binary_grid->getAccessor();
    for (int i = 0; i < length; i++) {
        openvdb::Coord loc = index_to_coord(i + start_voxel);
        acc.setValue(loc, 1.0);
    }
}

/**
 * Add a sequence of active voxels to the reorienter so it can update
 * its bounding box and centroid calculations
 */
void BinaryToLevelSet::update_reorienter(
        Reorienter& reorienter, int length, int start_voxel) {
    for (int i = 0; i < length; i++) {
        openvdb::Coord loc = index_to_coord(i + start_voxel);
        reorienter.add_voxel(loc);
    }
}

openvdb::Coord BinaryToLevelSet::index_to_coord(int index) {
    int y = index % dimensions.y();
    int z = (index / dimensions.y()) % dimensions.z();
    int x = index / (dimensions.y() * dimensions.z());
    return openvdb::Coord(x, y, z);
}

void BinaryToLevelSet::convert(float half_bandwidth, int smoothing_steps) {
    constexpr int CLOSING_STEPS = 1;
    constexpr int ADDITIONAL_DILATION_STEPS = 0;
    level_set = openvdb::tools::topologyToLevelSet(
        *binary_grid,
        half_bandwidth,
        CLOSING_STEPS,
        ADDITIONAL_DILATION_STEPS,
        smoothing_steps);
}

void BinaryToLevelSet::save(std::string filename) {
    // Store both grids
    openvdb::GridPtrVec grids;
    grids.push_back(level_set);

    // Write it to a file
    openvdb::io::File file(filename);
    file.write(grids);
    file.close();
}
