#ifndef MORPH_STATS_H
#define MORPH_STATS_H
#include <string>
#include <cstdlib>
#include <iostream>

class MorphStats {
    std::size_t cfl_count = 0;
    int time_steps = 0;
    int source_surface_count = 0;
    int target_surface_count = 0;
    int abs_diff_count = 0;
    double total_curvature = 0.0;
    double weighted_total_curvature = 0.0;
    double max_curvature = 0.0;
    double total_value = 0.0; 
    double weighted_total_value = 0.0;
    double total_energy = 0.0;
    double evolving_avg = 0.0;
    double src_tar_avg = 0.0;
    std::string source_name = "";
    std::string target_name = "";
public:
    static const int NUM_FIELDS = 14;
    /**
     * Write the column headers for the HTML report
     */
    static void write_header_row(std::ostream& stream);

    /**
     * Compute the mean of the total energy of two stat objects
     */
    static double mean(
        const MorphStats& first, const MorphStats& second);
    /**
     * Serialize to a HTML table row
     */
    void write_row(std::ostream& stream, double mean=-1.0) const;
};
#endif
