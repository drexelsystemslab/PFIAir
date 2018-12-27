#ifndef MORPH_STATS_H
#define MORPH_STATS_H
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>

/**
 * Stats for a morph in a single direction
 */
struct MorphStats {
    // Counters for computing averages =====================================

    // Keep a running total of surface voxels over time
    double total_surface_voxels;

    // Actual Stats ========================================================

    // Courrant-Friedrrichs-Lewy iterations from advecting the level sets
    int cfl_count = 0;
    // Number of time steps in the calculation
    int time_steps = 0;
    // Source surface voxels
    int source_surface_count = 0;
    // Target surface voxels
    int target_surface_count = 0;
    // |source surface count - target surface count|
    int abs_diff_count = 0;
    // (source surface count + target surface count) / 2
    double src_tar_avg = 0.0;
    double total_curvature = 0.0;
    double weighted_total_curvature = 0.0;
    double max_curvature = 0.0;
    double total_value = 0.0; 
    double weighted_total_value = 0.0;
    double total_energy = 0.0;
    double evolving_avg = 0.0;

    // Keep track of values of some of the above for each frame.
    // This way we can generate graphs in the reports
    std::vector<double> curve_delta_curvature;
    std::vector<double> curve_delta_value;
    // This one is cumulative maximum curvature
    std::vector<double> curve_max_curvature;
    // This is the max curvature per frame
    std::vector<double> curve_frame_max_curvature;
    std::vector<double> curve_cfl_iters;
    std::vector<double> curve_surface_voxels;

    // Source model without extension
    std::string source_name = "";
    // Target model without extension
    std::string target_name = "";
    // Number of fileds
    static constexpr int NUM_FIELDS = 14;

    // Weight for weighted_total_value
    static constexpr double VALUE_WEIGHT = 100.0;

    /**
     * Set source and target names at once
     */
    void set_names(std::string source, std::string target);

    /**
     * Count surface_voxels
     */
    void count_surface_voxels(double source_count, double target_count); 

    /**
     * Add CFL iterations from the latest call to advect
     */
    void add_cfl_iterations(int delta);
    
    /**
     * Add surface voxels to the total
     */
    void add_surface_voxels(int voxels);

    /**
     * Add one to the time steps
     */
    void increment_time();

    /**
     * Update total curvature and energy calculations
     */
    void update_energy(double delta_curvature, double delta_value);

    /**
     * Update the maximum curvature given the max curvature in the current
     * frame of the advection
     */
    void update_max_curvature(double frame_max_curvature);

    /**
     * Compute averages and other post-morph stats
     */
    void finalize_stats();
};

/**
 * Stats for a bi-directional morph
 */
struct MorphStatsPair {
    // forwards morph information
    MorphStats forwards;
    // Backwards morph information
    MorphStats backwards;

    MorphStatsPair() {}

    MorphStatsPair(MorphStats forwards, MorphStats backwards):
        forwards(forwards), backwards(backwards) {}

    /**
     * Return a new MorphStatsPair where target is the new source
     * and vice-versa. This is helpful when generating the all-pairs
     * report in Python land
     */
    MorphStatsPair swapped() const;

    /**
     * Compute the mean of the total energies of the two sets of stats
     */
    double average_energy();
};
#endif
