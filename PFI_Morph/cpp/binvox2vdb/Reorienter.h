#ifndef REORIENTER_H
#define REORIENTER_H

#include <openvdb/openvdb.h>

/**
 * Bounding box that starts empty but expands as we add voxels
 */
struct BBox {
    // Min and max coordinates
    openvdb::Vec3d min;
    openvdb::Vec3d max;

    /**
     * Default constructor. Set min to +inf and max to -inf
     */
    BBox();
    /**
     * Expand the box by marking an active voxel
     */
    void update(openvdb::Coord voxel);
    /**
     * Get the lengths of the bounding box in all three dimensions. This
     * is measured in index space, though the values are doubles
     */
    openvdb::Vec3d get_dimensions();
    /**
     * Get the center of the bounding box; that is half of the dimensions.
     * the result is measured in index space, though the value may be
     * fractional.
     */
    openvdb::Vec3d get_center();
    /**
     * sort the axes by length of the corresponding dimension of the bounding
     * box. Return a triple of indices that correspond to the axes [0, 1, 2]
     * = [x, y, z]
     *
     * For example, if the bounding box is longest in the z direction, 
     * second longest in the x direction, and shortest in y, this would
     * return (2, 0, 1)
     */
    openvdb::Vec3i sort_axes();
};

/**
 * This class handles reorienting the voxel models so that the longest
 * lengths are aligned the same way
 */
class Reorienter {
    BBox bbox;
    int num_voxels;
    openvdb::Vec3d sum;
    /**
     * basis vectors. These are used in compute_transform()
     */
    openvdb::Vec3d BASES[3] = {
        openvdb::Vec3d(1, 0, 0),
        openvdb::Vec3d(0, 1, 0),
        openvdb::Vec3d(0, 0, 1)
    };
public:
    Reorienter(): num_voxels(0), sum(0, 0, 0) {}
    /**
     * Tell the reorienter about an active voxel. This updates the
     * bounding box and centroid calculations
     */
    void add_voxel(openvdb::Coord voxel);
    /**
     * Compute the centroid of the 
     */
    openvdb::Vec3d get_centroid();
    /**
     * Compute the sign of each component of v, however, lean(0.0) = +1
     * so that result is always either +1 or -1
     */
    openvdb::Vec3d lean(openvdb::Vec3d v);
    /**
     * Compute the rotation matrix that meets the following criteria:
     *
     * 1. the longest axis gets sent to the x-axis such that the centroid's
     *    transformed x-coordinate points in the +x direction (> 0)
     * 2. Likewise, the second-longest axis should point in the y direction
     *    with the centroid pointing in the +y direction
     * 3. the shortest axis should be sent to the z-axis such that
     *    the orientation of x, y, z axes are preserved. This is done
     *    using an observation that rotations preserve cross products; the
     *    basis vectors computed in steps 1 and 2 are crossed to get the
     *    third one with the correct orientation
     */
    openvdb::math::Mat4d compute_rotation();
    /**
     * After populating the voxels, compute the transformation matrix
     * needed to center and orient the VDB grid
     */
    openvdb::math::Transform::Ptr compute_transform();
};

#endif
