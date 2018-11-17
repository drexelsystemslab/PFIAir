#include "LevelSet.h"
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetFilter.h>

LevelSet::LevelSet(
        const std::vector<openvdb::Vec3s>& vertices,
        const std::vector<openvdb::Vec3I>& indices_tri,
        const std::vector<openvdb::Vec4I>& indices_quad,
        bool is_open_mesh) {
    using namespace openvdb;

    math::Transform scale = math::Transform();
    scale.preScale(Vec3d(VOXEL_SIZE));

    if (is_open_mesh) {
        // Open meshes can only be read in as unsigned distance fields
        // However, if we use twice the bandwidth, we can convert it 
        // to a level set by a uniform subtraction.
        level_set = tools::meshToUnsignedDistanceField<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, 2.0 * HALF_BANDWIDTH);
        convert_unsigned_to_signed();
    } else {
        float width = HALF_BANDWIDTH;
        level_set = tools::meshToLevelSet<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, width);
        level_set->setGridClass(openvdb::GRID_LEVEL_SET);
    } 
}

openvdb::FloatGrid::Ptr LevelSet::get_level_set() {
    return level_set;
}

void LevelSet::morphological_opening() {
    openvdb::tools::LevelSetFilter<openvdb::FloatGrid> lsf(*level_set);
    double vs = level_set->voxelSize()[0];

    // Set number of normalizations
    lsf.setNormCount(5);
        
    // Erode by 1 voxel
    lsf.offset(vs);
    lsf.normalize();
        
    // Dilate by 1 voxel
    lsf.offset(-vs);
    lsf.normalize();
}

void LevelSet::convert_unsigned_to_signed() {
    // NOTE: If we need to squeeze out better performance,
    // look into openvdb::tools::foreach() to do these computations
    // using threading. This would involve moving some of the code
    // into a functor

    // Iterate over all values 
    typedef openvdb::FloatGrid::ValueAllIter AllIter;
    // Half the bandwidth is the new 0 point
    double zero_point = HALF_BANDWIDTH * VOXEL_SIZE;
    for (AllIter iter = level_set->beginValueAll(); iter; ++iter) {
        double current_val = *iter;
        // Subtracting values from the zero point uniformly
        // makes the region inside the new zero point negative and
        // the region outside positive.
        double new_val = zero_point - current_val;
        iter.setValue(new_val);
    }

    // Now the outer surface will have a distance of + half bandwidth
    // which is the same as the distance we used to set the zero point.
    // Update the background value for consistency.
    openvdb::tools::changeBackground(level_set->tree(), zero_point);

    // Mark this grid as a level set so OpenVDB doesn't complain.
    level_set->setGridClass(openvdb::GRID_LEVEL_SET);
}

void LevelSet::to_mesh(
        std::vector<openvdb::Vec3s>& out_vertices,
        std::vector<openvdb::Vec3I>& out_indices_tri,
        std::vector<openvdb::Vec4I>& out_indices_quad) {

    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(
        *level_set, out_vertices, out_indices_tri, out_indices_quad);
}

void LevelSet::save(std::string fname) {
    openvdb::io::File file(fname);
    openvdb::GridPtrVec grids;
    grids.push_back(level_set);
    file.write(grids);
    file.close();
}
