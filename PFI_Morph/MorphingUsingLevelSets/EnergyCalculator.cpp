#include "EnergyCalculator.h"
#include <cmath>

EnergyResults EnergyCalculator::compute_energy() {

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
    typedef openvdb::FloatGrid::ValueOnCIter IterType;
    for (IterType iter = prev_grid->cbeginValueOn(); iter; ++iter) {
        openvdb::Coord coord = iter.getCoord();
        if (iter.getValue() < 0 && prev_frame.is_surface_voxel(coord)) {
            // Compute the mean curvature for both frames
            double prev_curv = compute_mean_curvature(
                prev_map, prev_acc, coord); 
            double curr_curv = compute_mean_curvature(
                curr_map, curr_acc, coord);

            // Check for aliasing issues
            bool prev_curv_valid = std::isfinite(prev_curv);
            bool curr_curv_valid = std::isfinite(curr_curv);

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
            double prev_val = iter.getValue();
            double curr_val = curr_acc.getValue(coord);
            results.delta_value += std::abs(prev_val - curr_val);
        }
    }

    return results;

    /*
     * Pseudocode for updated energy calculation algorithm
     *
     * (maximum absolute mean cuvature for this pair of frames)
     * max_curvature = 0.0
     * delta_curvature = 0.0
     * delta_value = 0.0

     * For each active surface voxel in the previous frame:
     * - compute the mean curvature of prev @ current voxel 
     * - compute the mean curvature of curr @ current voxel 
     * - if both prev/curr mean curvature are infinite:
     *   - skip to the next voxel, we cannot get any meaningful info
     * - if one of prev/curr mean curvature is finite:
     *   - max_curvature = max(max_curvature, abs(finite mean curvature))
     *   - curv_diff = |finite mean curvature|
     *   - delta_curvature += curv_diff
     * - if both prev/curr mean curvature are finite:
     *   - max_curvature = max(
     *         max_curvature, abs(prev mean curvature), abs(curr mean curvature))
     *   - curv_diff = |prev mean curvature - curr mean curvature|
     *   - delta_curvature += curv_diff
     * - delta_value = |prev.value[coord] - curr.value[coord]|
     *   
     * Return the following:
     *
     * results.delta_value: 
     * results.delta_curvature: the change in mean curvature during the morph
     *      from prev -> current
     * results.max_curvature: maximum mean curvature *for this pair of frames*
     */ 
}

double EnergyCalculator::compute_mean_curvature(
        const MapBase::ConstPtr& map, 
        const openvdb::FloatGrid::ConstAccessor& acc, 
        const openvdb::Coord& coord) {
    double alpha;
    double beta;
    mean_curv.compute(*map, acc, coord, alpha, beta);
    if (beta != 0.0)
        return alpha / beta;
    else
        return std::numeric_limits<double>::infinity();
}
