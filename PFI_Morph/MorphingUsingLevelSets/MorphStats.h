#ifndef MORPH_STATS_H
#define MORPH_STATS_H
#include <string>
#include <cstdlib>

class MorphStats {
    std::size_t CFL_count = 0;
    int time_steps = 0;
    int source_surface_count = 0;
    int target_surface_count = 0;
    int abs_diff_count = 0;
    double total_curv = 0.0;
    double weighted_total_curv = 0.0;
    double max_curv = 0.0;
    double total_val = 0.0; 
    double weighted_total_val = 0.0;
    double total_energy = 0.0;
    double mean = 0.0;
    double evol_avg = 0.0;
    double src_tar_avg = 0.0;
    std::string source_name;
    std::string target_name;
public:
};
#endif
