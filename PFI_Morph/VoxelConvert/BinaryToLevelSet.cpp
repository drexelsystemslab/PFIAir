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

    for (const RunLength& run : data) {
        // Value is either 0 to 1
        int value = run.first;
        int length = run.second;

        if (value == 0) {
            // Don't store 0 values in the level set. This is necessary
            // for topologyToLevelSet() to work properly
            voxel_count += length;
        } else {
            populate_run(length, voxel_count);
            voxel_count += length;
        }
    }

    // Set the scale and origin =============================
    using namespace openvdb::math;

    // scale the grid to the right size, then translate so the model is
    // centered on the origin
    Mat4d T = Mat4d::translation(translation);
    Mat4d S = Mat4d::identity();
    S.setToScale(Vec3d(scale, scale, scale));
    Mat4d TS = T * S;

    std::cout << T.str() << std::endl << std::endl << S.str() << std::endl;

    // Set a transformation
    Transform::Ptr xform = Transform::createLinearTransform(TS);
    binary_grid->setTransform(xform);
    //grid->setTransform(math::Transform::createLinearTransform(voxel_size));
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

openvdb::Coord BinaryToLevelSet::index_to_coord(int index) {
    int y = index % dimensions.y();
    int z = (index / dimensions.y()) % dimensions.z();
    int x = index / (dimensions.y() * dimensions.z());
    return openvdb::Coord(x, y, z);
}

void BinaryToLevelSet::convert() {
    level_set = openvdb::tools::topologyToLevelSet(*binary_grid);
}

void BinaryToLevelSet::save(std::string filename) {
    // Store both grids
    openvdb::GridPtrVec grids;
    grids.push_back(binary_grid);
    grids.push_back(level_set);

    // Write it to a file
    openvdb::io::File file(filename);
    file.write(grids);
    file.close();
}
