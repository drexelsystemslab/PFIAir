#include <openvdb/openvdb.h>
#include <openvdb/tools/TopologyToLevelSet.h>
#include <iostream>
#include <functional>
using namespace openvdb;

// Signed distance function
typedef std::function<float (const Vec3d&)> SDF;


SDF sphere(float radius) {
    return [&radius](const Vec3d& p) {
        return p.length() - radius;
    };
}

void make_level_set(
        FloatGrid::Ptr grid,
        const CoordBBox& index_bbox,
        double voxel_size,
        SDF dist_field) {
    FloatGrid::Accessor accessor = grid->getAccessor();
    Coord min_coord = index_bbox.min();
    Coord max_coord = index_bbox.max();

    for (Int32 i = min_coord.x(); i <= max_coord.x(); i++) {
        for (Int32 j = min_coord.y(); j <= max_coord.y(); j++) {
            for (Int32 k = min_coord.z(); k <= max_coord.z(); k++) {
                // Transform from index -> world space
                Vec3d point(i, j, k);
                point *= voxel_size;

                float distance = dist_field(point);

                accessor.setValue(Coord(i, j, k), distance);
            }
        }
    }

    grid->setTransform(math::Transform::createLinearTransform(voxel_size));
    grid->setGridClass(GRID_LEVEL_SET);
}

void make_binary_grid(
        FloatGrid::Ptr grid,
        const CoordBBox& index_bbox,
        double voxel_size,
        SDF dist_field) { 
    FloatGrid::Accessor accessor = grid->getAccessor();
    Coord min_coord = index_bbox.min();
    Coord max_coord = index_bbox.max();

    for (Int32 i = min_coord.x(); i <= max_coord.x(); i++) {
        for (Int32 j = min_coord.y(); j <= max_coord.y(); j++) {
            for (Int32 k = min_coord.z(); k <= max_coord.z(); k++) {
                // Transform from index -> world space
                Vec3d point(i, j, k);
                point *= voxel_size;

                float distance = dist_field(point);

                if (distance < 0.0)
                    accessor.setValue(Coord(i, j, k), 1.0);
            }
        }
    }

    grid->setTransform(math::Transform::createLinearTransform(voxel_size));
}

FloatGrid::Ptr binary_to_level_set(FloatGrid::Ptr grid) {
    return tools::topologyToLevelSet(*grid);
}

void save_grid(FloatGrid::Ptr grid, std::string fname) {
    GridPtrVec grids;
    grids.push_back(grid);
    io::File file(fname);
    file.write(grids);
    file.close();
}

int main() {
    initialize();

    FloatGrid::Ptr grid = FloatGrid::create(2.0);
    CoordBBox index_bbox(Coord(-64, -64, -64), Coord(63, 63, 63));

    make_binary_grid(grid, index_bbox, 1.0 / 128.0, sphere(0.49));

    grid->setName("LevelSetSphereBinary");

    // Save the file
    save_grid(grid, "output/convert_voxels/sphere_test_bin.vdb");

    FloatGrid::Ptr ls_grid = binary_to_level_set(grid);
    save_grid(ls_grid, "output/convert_voxels/converted_test.vdb");
}
