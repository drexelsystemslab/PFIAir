#include "MorphStats.h"


void MorphStats::set_names(std::string source, std::string target) {
    source_name = source;
    target_name = target;
}

void MorphStats::count_surface_voxels(
        double source_count, double target_count) {
    // Store the counts
    source_surface_count = source_count;
    target_surface_count = target_count; 
    
    // Compute absolute difference and average
    abs_diff_count = std::abs(source_count - target_count);
    src_tar_avg = (source_count + target_count) / 2; 

    // Initialize the total surface voxels to that of the source count
    total_surface_voxels = source_surface_count;

    // Only add the source surface count to the curve.
    curve_surface_voxels.push_back(source_surface_count);
}

void MorphStats::add_cfl_iterations(int delta) {
    cfl_count += delta;
    curve_cfl_iters.push_back(delta);
}

void MorphStats::add_surface_voxels(int voxels) {
    total_surface_voxels += voxels;
    curve_surface_voxels.push_back(voxels);
}

void MorphStats::increment_time() {
    time_steps++;
}

void MorphStats::update_energy(double delta_curvature, double delta_value) {
    total_energy += delta_curvature + delta_value;
    total_curvature += delta_curvature;
    total_value += delta_value;

    curve_delta_curvature.push_back(delta_curvature);
    curve_delta_value.push_back(delta_value);
}

void MorphStats::update_max_curvature(double frame_max_curvature) {
    if (frame_max_curvature > max_curvature)
        max_curvature = frame_max_curvature;

    curve_frame_max_curvature.push_back(frame_max_curvature);
    curve_max_curvature.push_back(max_curvature);
}

void MorphStats::finalize_stats() {
    evolving_avg = total_surface_voxels / (time_steps + 1);
    weighted_total_curvature = total_curvature / evolving_avg;
    weighted_total_value = (total_value / evolving_avg) * VALUE_WEIGHT;
    total_energy = weighted_total_curvature + weighted_total_value;
}

// ==============================================================

double MorphStatsPair::average_energy() {
    return (forwards.total_energy + backwards.total_energy) / 2;
}

MorphStatsPair MorphStatsPair::swapped() const {
    // Flip the source/target
    return MorphStatsPair(backwards, forwards);
}
