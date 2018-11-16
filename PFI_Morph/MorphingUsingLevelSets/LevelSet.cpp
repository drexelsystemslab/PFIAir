#include "LevelSet.h"
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>

LevelSet::LevelSet(
        const std::vector<openvdb::Vec3s>& vertices,
        const std::vector<openvdb::Vec3I>& indices_tri,
        const std::vector<openvdb::Vec4I>& indices_quad,
        bool is_open_mesh) {
    using namespace openvdb;

    math::Transform scale = math::Transform();
    scale.preScale(Vec3d(VOXEL_SIZE));

    if (is_open_mesh) {
        // Open meshes can only be read 
        level_set = tools::meshToUnsignedDistanceField<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, 2.0 * HALF_BANDWIDTH);
        convert_unsigned_to_signed(2.0 * HALF_BANDWIDTH);
    } else {
        float width = HALF_BANDWIDTH;
        level_set = tools::meshToLevelSet<FloatGrid>(
            scale, vertices, indices_tri, indices_quad, width);
    } 
}

void LevelSet::morphological_opening() {
    std::cout << "TODO: Morphological opening" << std::endl;
}

void LevelSet::convert_unsigned_to_signed(float bandwidth) {
    std::cout <<  "TODO: Convert unsigned -> signed" << std::endl;
}

void LevelSet::to_mesh(
        std::vector<openvdb::Vec3s>& out_vertices,
        std::vector<openvdb::Vec3I>& out_indices_tri,
        std::vector<openvdb::Vec4I>& out_indices_quad) {

    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(
        *level_set, out_vertices, out_indices_tri, out_indices_quad);
}
