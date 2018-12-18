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

    // Mean curvature calculator
    MeanCurvature<MapBase, DDScheme::CD_SECOND, DScheme::CD_2ND> mean_curv;
public:
    EnergyCalculator(const LevelSet& prev_frame, const LevelSet& curr_frame):
        prev_frame(prev_frame),
        curr_frame(curr_frame) {}

    EnergyResults compute_energy();

    /**
     * Wrapper around mean_curv.compute() that turns zero denominator into
     * double infinity
     */
    double compute_mean_curvature(
        const MapBase::ConstPtr& map, 
        const openvdb::FloatGrid::ConstAccessor& acc, 
        const openvdb::Coord& coord);
    
};

#endif
