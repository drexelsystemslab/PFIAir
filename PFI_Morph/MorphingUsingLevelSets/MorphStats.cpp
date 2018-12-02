#include "MorphStats.h"

//constexpr int MorphStats::NUM_FIELDS;
//constexpr double MorphStats::VALUE_WEIGHT;

void MorphStats::write_row(std::ostream& stream, double mean) const {
    stream 
        << "            <td colspan=" << NUM_FIELDS << ">" << std::endl
        << source_name << "-" << target_name << std::endl
        << "            </td>" << std::endl
        << "        </tr>" << std::endl
        << "        <tr>" << std::endl
        << "            <td>" << cfl_count << "</td>" << std::endl
        << "            <td>" << time_steps << "</td>" << std::endl
        << "            <td>" << source_surface_count << "</td>" << std::endl
        << "            <td>" << target_surface_count << "</td>" << std::endl
        << "            <td>" << abs_diff_count << "</td>" << std::endl
        << "            <td>" << evolving_avg << "</td>" << std::endl
        << "            <td>" << src_tar_avg << "</td>" << std::endl
        << "            <td>" << total_curvature << "</td>" << std::endl
        << "            <td>" << max_curvature << "</td>" << std::endl
        << "            <td>" << weighted_total_curvature << "</td>" 
        << std::endl
        << "            <td>" << total_value << "</td>" << std::endl
        << "            <td>" << weighted_total_value << "</td>" << std::endl
        << "            <td>" << total_energy << "</td>" << std::endl;

    if (mean >= 0.0)
        stream << "            <td>" << mean << "</td>" << std::endl;
    else
        stream << "            <td></td>" << std::endl;

    stream << "        </tr>" << std::endl;
}

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
}

void MorphStats::add_cfl_iterations(int delta) {
    cfl_count += delta;
}

void MorphStats::increment_time() {
    time_steps++;
}

void MorphStats::update_energy(double delta_curvature, double delta_value) {
    total_energy += delta_curvature + delta_value;
    total_curvature +=  delta_curvature;
    total_value += delta_value;
}

void MorphStats::update_max_curvature(double frame_max_curvature) {
    if (frame_max_curvature > max_curvature)
        max_curvature = frame_max_curvature;
}

void MorphStats::finalize_stats() {
    evolving_avg = total_surface_voxels / (time_steps + 1);
    weighted_total_curvature = total_curvature / evolving_avg;
    weighted_total_value = (total_value / evolving_avg) * VALUE_WEIGHT;
    total_energy = weighted_total_curvature + weighted_total_value;
}

void MorphStats::write_header_row(std::ostream& stream) {
    stream
        << "        <tr>" << std::endl
        << "            <th>Image</th>" << std::endl
        << "            <th>CFL Iterations</th>" << std::endl
        << "            <th>Time steps</th>" << std::endl
        << "            <th>Source voxel count</th>" << std::endl
        << "            <th>Target voxel count</th>" << std::endl
        << "            <th>Abs diff</th>" << std::endl
        << "            <th>Evolving average</th>" << std::endl
        << "            <th>Source-target average</th>" << std::endl
        << "            <th>Total Curvature</th>" << std::endl
        << "            <th>Max Curvature</th>" << std::endl
        << "            <th>Curvature / unit area</th>" << std::endl
        << "            <th>Total Value</th>" << std::endl
        << "            <th>(Value / unit area) * 100</th>" << std::endl
        << "            <th>Total Energy</th>" << std::endl
        << "            <th>Mean</th>" << std::endl
        << "        </tr>" << std::endl;
}

// ==============================================================

double MorphStatsPair::average_energy() {
    return (forwards.total_energy + backwards.total_energy) / 2;
}
