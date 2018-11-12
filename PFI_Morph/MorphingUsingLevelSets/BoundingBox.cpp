#include "BoundingBox.h"
#include <algorithm>

const int BoundingBox::X;
const int BoundingBox::Y;
const int BoundingBox::Z;

const int BoundingBox::MAX_LENGTH;
const int BoundingBox::MID_LENGTH;
const int BoundingBox::MIN_LENGTH;

Eigen::Vector3d BoundingBox::get_min_coords() const {
    return min_coords;
}

Eigen::Vector3d BoundingBox::get_max_coords() const {
    return max_coords;
}

Eigen::Vector3d BoundingBox::get_dimensions() const {
    Eigen::Array3d diff = max_coords - min_coords;
    return diff.abs();
}

Eigen::Vector3d BoundingBox::get_sorted_dimensions() const {
    Eigen::Vector3d dims = get_dimensions();

    // Sort the three dimensions
    double maximum = dims(X);
    double middle = dims(Y);
    double minimum = dims(Z);
    if (maximum < middle)
        std::swap(maximum, middle);
    if (maximum < minimum)
        std::swap(maximum, minimum);
    if (middle < minimum)
        std::swap(middle, minimum);

    // Return in descending order
    return Eigen::Vector3d(maximum, middle, minimum);
}

double BoundingBox::get_max_length() const {
    return get_sorted_dimensions()(MAX_LENGTH);
}

Eigen::Vector3d BoundingBox::get_center() const{
    // Compute the midpoint of the max and min coords
    // to get the centroid.
    return (max_coords + min_coords) / 2.0;
}


// ==============================================================

std::ostream& operator<<(std::ostream& stream, const BoundingBox& bbox) {
    stream << "BoundingBox:" << std::endl
        << "min: " << bbox.get_min_coords().transpose() << std::endl
        << "max: " << bbox.get_max_coords().transpose() << std::endl
        << "center: " << bbox.get_center().transpose() << std::endl
        << "sorted lengths: " << bbox.get_sorted_dimensions().transpose() 
        << std::endl;
    return stream;
}
