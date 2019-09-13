#ifndef ENERGY_CALCULATOR_H
#define ENERGY_CALCULATOR_H
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/openvdb.h>
#include "LevelSet.h"
using namespace openvdb::math;

/**
 * Morph::calculate_energy() comuptes a few different values,
 * so pack them up in a struct.
 */
struct EnergyResults {
    // max(abs(mean curvature)) for this pair of frames.
    double max_curvature;

    // Change in mean curvature for this pair of frames
    double delta_curvature;

    // total change in distance field values from previous to 
    // current frame. This should correlate with change in volume
    double delta_value;
};

class EnergyCalculator {
    // Level sets
    const LevelSet& prev_frame;
    const LevelSet& curr_frame; 
public:
    EnergyCalculator(const LevelSet& prev_frame, const LevelSet& curr_frame):
        prev_frame(prev_frame),
        curr_frame(curr_frame) {}

    /**
     * Compute the energy for this pair of frames.
     *
     * pass in the maximum absolute mean curvature of any surface point
     * of either source or target model
     */
    EnergyResults compute_energy(double max_curv);

    /**
     * Compute the maximum absolute mean curvature across an entire
     * level set. This is used to set a reasonable maximum
     * curvature threshold above which curvature values are ignored
     */
    static double compute_max_curvature(const LevelSet& level_set);

    /**
     * Wrapper around mean_curv.compute() that turns zero denominator into
     * double infinity
     */
    static double compute_mean_curvature(
        const MapBase::ConstPtr& map, 
        const openvdb::FloatGrid::ConstAccessor& acc, 
        const openvdb::Coord& coord);
    
};

#endif
