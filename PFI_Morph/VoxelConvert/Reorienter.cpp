#include "Reorienter.h"
#include <limits>
#include <iostream>

BBox::BBox() {
    double inf = std::numeric_limits<double>::infinity();
    // Start with an empty box
    min = openvdb::Vec3d(inf, inf, inf);
    max = openvdb::Vec3d(-inf, -inf, -inf);
}

void BBox::update(openvdb::Coord voxel) {
    double x = voxel.x();
    double y = voxel.y();
    double z = voxel.z();

    // Expand min coordinates
    if (x < min.x())
        min.x() = x;
    if (y < min.y())
        min.y() = y;
    if (z < min.z())
        min.z() = z;

    // Expand max coordinates
    if (x > max.x())
        max.x() = x;
    if (y > max.y())
        max.y() = y;
    if (z > max.z())
        max.z() = z;
}

openvdb::Vec3d BBox::get_dimensions()  {
    openvdb::Vec3d result(max);
    result -= min;
    return result;
}

openvdb::Vec3d BBox::get_center() {
    openvdb::Vec3d result = get_dimensions();
    result /= 2;
    return result;
}

openvdb::Vec3i BBox::sort_axes() {
    int indices[] = {0, 1, 2};

    openvdb::Vec3d dim_vec = get_dimensions();
    double dims[] = {dim_vec.x(), dim_vec.y(), dim_vec.z()};

    // Insertion sort indices by length of dimensions =======================
    // Annoying to type out, but it's not enough to warrant an entire
    // sorting functor
    if (dims[1] > dims[0]) {
        std::swap(dims[0], dims[1]);
        std::swap(indices[0], indices[1]);
    }

    if (dims[2] > dims[1]) {
        std::swap(dims[1], dims[2]);
        std::swap(indices[1], indices[2]);
    }

    if (dims[1] > dims[0]) {
        std::swap(dims[0], dims[1]);
        std::swap(indices[0], indices[1]);
    }
    // ==================================================================

    // Package the results in an integer vector
    return openvdb::Vec3i(indices[0], indices[1], indices[2]); 
}

// ===================================================

void Reorienter::add_voxel(openvdb::Coord voxel) {
    // Update the bounding box
    bbox.update(voxel);

    // Update the centroid sum and count
    num_voxels++;
    sum += voxel.asVec3d(); 
}

openvdb::Vec3d Reorienter::get_centroid() {
    openvdb::Vec3d result(sum);
    result /= num_voxels;
    return result;
}

openvdb::Vec3d Reorienter::lean(openvdb::Vec3d v) {
    double x = (v.x() >= 0.0) ? 1 : -1;
    double y = (v.y() >= 0.0) ? 1 : -1;
    double z = (v.z() >= 0.0) ? 1 : -1;
    return openvdb::Vec3d(x, y, z);
}

openvdb::math::Mat4d Reorienter::compute_rotation() {
    // Compute the centroid relative to the center of the bounding box
    openvdb::Vec3d bbox_center = bbox.get_center();
    openvdb::Vec3d centroid_rel = get_centroid();
    centroid_rel -= bbox_center;

    // Now see which side of the bbox center the centroid leans
    // by computing the sign of each component.
    openvdb::Vec3d centroid_lean = lean(centroid_rel);
    //std::cout << "centroid" << centroid_lean << std::endl;

    // Sort the bounding box components but get the indices in [0, 1, 2] of
    // which axes they refer to. indices.x() denotes which axis is the longest
    openvdb::Vec3i indices = bbox.sort_axes();

    // Compute the inverse rotation matrix

    // 1. The longest axis is mapped to the x axis. such that the centroid
    // leans towards +x
    openvdb::Vec3d Rx = BASES[indices.x()] * centroid_lean[indices.x()];

    // 2. Likewise, the second longest axis is mapped to the y axis
    // such that the centroid leans towards +y
    openvdb::Vec3d Ry = BASES[indices.y()] * centroid_lean[indices.y()];

    // 3. The shortest axis must point along the z axis, but the direction
    // must be picked to preserve orientation of the model. Since
    // proper rotations preserve cross products, a cross product of Rx
    // and Ry does the trick
    openvdb::Vec3d Rz = Rx.cross(Ry);

    // Return a rotation matrix
    openvdb::math::Mat3d rotation(Rx, Ry, Rz, false);
    //std::cout << Rx << std::endl;
    //std::cout << Ry << std::endl;
    //std::cout << rotation << std::endl; 

    // Convert it into a 4D matrix with the homogeneous coordinate
    openvdb::math::Mat4d result;
    result.setIdentity();
    result.setMat3(rotation);

    return result;
}

openvdb::math::Transform::Ptr Reorienter::compute_transform() {
    // Create the identity transformation
    openvdb::math::Transform::Ptr xform = 
        openvdb::math::Transform::createLinearTransform();

    /**
     * The transformations are applied in this order (which is not the
     * usual order for affine transformations)
     * 1. Translate the grid so the center of the bounding box is at the
     *    origin. This is done by a pre-translation
     * 2. Rotate the grid so that longest dimension of the bounding box
     *    is in the x direction. There are other constraints, see
     *    compute_rotation() for details
     */
    xform->preTranslate(-bbox.get_center());
    xform->postMult(compute_rotation());
    return xform;
}
