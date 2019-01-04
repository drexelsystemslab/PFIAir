#include "LevelSet.h"
#include <stdexcept>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetFilter.h>

// SurfaceIterator ====================================================

void SurfaceIterator::begin() {
    iter = grid->cbeginValueOn();
}

SurfaceIterator::operator bool() {
    return static_cast<bool>(iter);
}

openvdb::Coord SurfaceIterator::get_coord() {
    return iter.getCoord();
}

double SurfaceIterator::get_value() {
    return iter.getValue();
}

openvdb::FloatGrid::ValueOnCIter SurfaceIterator::vdb_iter() {
    return iter;
}

void SurfaceIterator::operator++(int) {
    // Always advance at least one voxel
    ++iter;

    // If we are at a non-surface voxel,
    // advance again
    while (iter && !is_surface_voxel())
        ++iter;
}

bool SurfaceIterator::is_surface_voxel() {
    openvdb::Coord coord = iter.getCoord();

    // We only want to count voxels that are slightly negative of
    // zero with a neighbor that is positive
    if (iter.getValue() >= 0)
        return false;
    
    // While the exact coords vary from voxel to voxel,
    // the 6 directions are the same
    constexpr int NUM_DIRS = 6;
    const openvdb::Coord neighbor_directions[NUM_DIRS] = {
        openvdb::Coord(-1, 0, 0),
        openvdb::Coord(+1, 0, 0),
        openvdb::Coord(0, -1, 0),
        openvdb::Coord(0, +1, 0),
        openvdb::Coord(0, 0, -1),
        openvdb::Coord(0, 0, +1)
    };

    // Check each direction until we find a positive neighbor
    openvdb::FloatGrid::ConstAccessor acc = grid->getConstAccessor();
    for (int i = 0; i < NUM_DIRS; i++) {
        // Neighbor = position + offset
        openvdb::Coord neighbor = coord + neighbor_directions[i];

        // If we found a positive neighbor, we can stop and return true
        // since we are on the edge
        if (acc.getValue(neighbor) > 0) 
            return true;
    }

    // We are not on the edge
    return false; 
}

// Level Set =============================================================

LevelSet::LevelSet(std::string fname, double half_bandwidth):
        half_bandwidth(half_bandwidth) {
    openvdb::io::File file(fname);
    openvdb::GridBase::Ptr baseGrid;

    file.open();
    typedef openvdb::io::File::NameIterator NameIter;
    for (NameIter it = file.beginName(); it != file.endName(); ++it) {
        baseGrid = file.readGrid(it.gridName());
    }    
    file.close();

    level_set = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
}

LevelSet::LevelSet(
        const std::vector<openvdb::Vec3s>& vertices,
        const std::vector<openvdb::Vec3I>& indices_tri,
        const std::vector<openvdb::Vec4I>& indices_quad,
        bool is_open_mesh,
        double half_bandwidth): half_bandwidth(half_bandwidth) {
    using namespace openvdb;

    math::Transform scale = math::Transform();
    scale.preScale(Vec3d(VOXEL_SIZE));

    if (is_open_mesh) {
        // Open meshes can only be read in as unsigned distance fields.
        // However, if we offset the distance field by a constant amount
        // (3 voxels) we now have an inside which is negative and an outside
        // which is positive. The inner size will always be the default
        // bandwidth
        double inside_width = HALF_BANDWIDTH;
        // However, the outside bandwidth may be much larger to help prevent
        // aliasing.
        double outside_width = half_bandwidth;
        double full_width = inside_width + outside_width;

        // Convert to an *unsigned* distance field with a width of the
        // entire desired bandwidth
        level_set = tools::meshToUnsignedDistanceField<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, full_width);

        // Set the 0 surface 3 voxels away so we have a level set.
        convert_unsigned_to_signed();
    } else {
        float width = half_bandwidth;
        level_set = tools::meshToLevelSet<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, width);
        level_set->setGridClass(openvdb::GRID_LEVEL_SET);
    } 
}

openvdb::FloatGrid::Ptr LevelSet::get_level_set() {
    return level_set;
}

openvdb::FloatGrid::ConstPtr LevelSet::get_level_set() const {
    return level_set;
}

void LevelSet::morphological_opening() {
    openvdb::tools::LevelSetFilter<openvdb::FloatGrid> lsf(*level_set);
    double voxel_size = level_set->voxelSize()[0];

    // Set number of normalizations
    lsf.setNormCount(NORM_COUNT);
        
    // Erode by 1 voxel
    lsf.offset(OPENING_VOXELS * voxel_size);
    lsf.normalize();
        
    // Dilate by 1 voxel
    lsf.offset(-OPENING_VOXELS * voxel_size);
    lsf.normalize();
}

int LevelSet::count_surface_voxels() const {
    int count = 0;
    SurfaceIterator surf_iter = get_surface_iterator();
    for (surf_iter.begin(); surf_iter; surf_iter++) {
        count++;
    }
    return count;
} 

double LevelSet::get_voxel_size() const {
    return level_set->voxelSize()[0];
}

void LevelSet::convert_unsigned_to_signed() {
    // NOTE: If we need to squeeze out better performance,
    // look into openvdb::tools::foreach() to do these computations
    // using threading. This would involve moving some of the code
    // into a functor

    // 3 voxels away from the surface will be the inflated level set,
    // even if the half bandwidth is larger
    double zero_point = HALF_BANDWIDTH * VOXEL_SIZE;

    // Iterate over all values 
    typedef openvdb::FloatGrid::ValueAllIter AllIter;
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

LevelSet LevelSet::deep_copy() const { 
    using openvdb::FloatGrid;
    FloatGrid::Ptr copy = openvdb::deepCopyTypedGrid<FloatGrid>(level_set);
    return LevelSet(copy);
}

void LevelSet::save(std::string fname) const {
    openvdb::io::File file(fname);
    openvdb::GridPtrVec grids;
    grids.push_back(level_set);
    file.write(grids);
    file.close();
}

SurfaceIterator LevelSet::get_surface_iterator() const {
    return SurfaceIterator(level_set);
}
