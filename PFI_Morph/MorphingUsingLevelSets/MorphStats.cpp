#include "MorphStats.h"

const int MorphStats::NUM_FIELDS;

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


double MorphStats::mean(const MorphStats& first, const MorphStats& second) {
    return (first.total_energy + second.total_energy) / 2;
}
