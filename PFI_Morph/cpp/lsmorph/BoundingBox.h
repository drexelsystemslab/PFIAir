#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
#include <eigen3/Eigen/Dense>
#include <iostream>

class BoundingBox {
    Eigen::Vector3d min_coords;
    Eigen::Vector3d max_coords;
public:
    // Components of a vector in R^3
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;

    // Components of the sorted length vector
    static const int MAX_LENGTH = 0;
    static const int MID_LENGTH = 1;
    static const int MIN_LENGTH = 2;

    BoundingBox() {}

    BoundingBox(Eigen::Vector3d min_coords, Eigen::Vector3d max_coords):
        min_coords(min_coords), max_coords(max_coords) {}

    // Getters
    Eigen::Vector3d get_min_coords() const;
    Eigen::Vector3d get_max_coords() const;

    /**
     * Get the dimensions of the bounding box
     */
    Eigen::Vector3d get_dimensions() const;
    
    /**
     * Get the dimensions of the bounding box sorted in
     * descending order from biggest to smallest
     */
    Eigen::Vector3d get_sorted_dimensions() const;

    /**
     * Get the maximumm length from the sorted dimensions
     */
    double get_max_length() const;

    /**
     * Get the center of the bounding box
     */
    Eigen::Vector3d get_center() const;
};

std::ostream& operator<<(std::ostream& stream, const BoundingBox& bbox);
#endif
