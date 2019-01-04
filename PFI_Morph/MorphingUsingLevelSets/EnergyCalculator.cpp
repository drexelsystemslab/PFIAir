#include "EnergyCalculator.h"
#include <cmath>


EnergyResults EnergyCalculator::compute_energy(double max_curv) {
    EnergyResults results;
    results.max_curvature = 0.0;
    results.delta_curvature = 0.0;
    results.delta_value = 0.0;

    // get the low-level grids
    openvdb::FloatGrid::ConstPtr prev_grid = prev_frame.get_level_set();
    openvdb::FloatGrid::ConstPtr curr_grid = curr_frame.get_level_set();

    // And the accessors
    openvdb::FloatGrid::ConstAccessor prev_acc = prev_grid->getAccessor();
    openvdb::FloatGrid::ConstAccessor curr_acc = curr_grid->getAccessor();

    // and MapBases
    MapBase::ConstPtr prev_map = prev_grid->transform().baseMap();
    MapBase::ConstPtr curr_map = curr_grid->transform().baseMap();

    // Iterate over surface voxels in the previous frame
    SurfaceIterator surf_iter = prev_frame.get_surface_iterator();
    for (surf_iter.begin(); surf_iter; surf_iter++) {
        openvdb::Coord coord = surf_iter.get_coord();

        // Compute the mean curvature for both frames
        double prev_curv = compute_mean_curvature(prev_map, prev_acc, coord); 
        double curr_curv = compute_mean_curvature(curr_map, curr_acc, coord);

        // Check for aliasing issues
        bool prev_curv_valid = std::isfinite(prev_curv) 
            && std::abs(prev_curv) < max_curv;
        bool curr_curv_valid = std::isfinite(curr_curv)
            && std::abs(curr_curv) < max_curv;

        // Handle surface curvature changes
        if (prev_curv_valid && curr_curv_valid) {
            // Both are valid, so compare the values

            // Max curvature: max of both values and the old maximum
            double abs_prev = std::abs(prev_curv);
            double abs_curr = std::abs(curr_curv);
            results.max_curvature = std::max({
                results.max_curvature, abs_prev, abs_curr});

            // Delta curvature: absolute difference of curvatures
            results.delta_curvature += std::abs(prev_curv - curr_curv);
        } else if (prev_curv_valid) {
            // Only the voxel in the previous frame is valid. Use its
            // values.
            // we only care about the magnitude of the curvature change
            // so take the absolute value
            double abs_curvature = std::abs(prev_curv);
            results.max_curvature = std::max(
                results.max_curvature, abs_curvature);
            results.delta_curvature += abs_curvature;
        } else if (curr_curv_valid) {
            // current frame only is handled symmetrically
            double abs_curvature = std::abs(curr_curv);
            results.max_curvature = std::max(
                results.max_curvature, abs_curvature);
            results.delta_curvature += abs_curvature;
        } else {
            // Neither is valid, so skip this voxel
            continue;
        }

        // Handle changes in value, which should be roughly
        // related to changes in volume
        double prev_val = surf_iter.get_value();
        double curr_val = curr_acc.getValue(coord);
        results.delta_value += std::abs(prev_val - curr_val);
    }

    return results;
}

double EnergyCalculator::compute_max_curvature(const LevelSet& level_set) {
    MeanCurvature<MapBase, DDScheme::CD_SECOND, DScheme::CD_2ND> mean_curv;
    openvdb::FloatGrid::ConstPtr grid = level_set.get_level_set();
    openvdb::FloatGrid::ConstAccessor acc = grid->getAccessor();
    MapBase::ConstPtr map = grid->transform().baseMap();

    double max_curv = -std::numeric_limits<double>::infinity();

    // Iterate over surface voxels
    SurfaceIterator surf_iter = level_set.get_surface_iterator();
    for (surf_iter.begin(); surf_iter; surf_iter++) {
        // Compute the absolute mean curvature at this point
        double curv = compute_mean_curvature(map, acc, surf_iter.get_coord());
        curv = std::abs(curv);
            
        // If we found a new maximum curvature that is finite, 
        // update the value
        if (std::isfinite(curv) && curv > max_curv)
            max_curv = curv;
    }

    return max_curv;
}

double EnergyCalculator::compute_mean_curvature(
        const MapBase::ConstPtr& map, 
        const openvdb::FloatGrid::ConstAccessor& acc, 
        const openvdb::Coord& coord) {
    MeanCurvature<MapBase, DDScheme::CD_SECOND, DScheme::CD_2ND> mean_curv;
    double alpha;
    double beta;
    mean_curv.compute(*map, acc, coord, alpha, beta);
    if (beta != 0.0)
        return alpha / beta;
    else
        return std::numeric_limits<double>::infinity();
}
