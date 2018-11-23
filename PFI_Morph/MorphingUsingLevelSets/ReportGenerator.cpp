#include "ReportGenerator.h"
#include <fstream>
#include <algorithm>

void ReportGenerator::add_row(MorphStats forward, MorphStats backward) {
    StatPair pair = std::make_pair(forward, backward);
    stat_pairs.push_back(pair);
}

void ReportGenerator::write_report() {
    std::ofstream file(fname);
    if (!file)
        throw new std::runtime_error("Cannot open " + fname);

    // Set the local on the stream so we can format numbers properly
    file.imbue(std::locale(""));

    write_header(file);

    write_table(file);

    write_footer(file);

    file.close();
}

void ReportGenerator::write_header(std::ostream& stream) {
    stream 
        << "<html>" << std::endl
        << "<head>" << std::endl
        << "    <link rel='stylesheet' href='style.css'>" << std::endl
        << "    <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css' integrity='sha384-/Y6pD6FV/Vv2HJnA6t+vslU6fwYXjCFtcEpHbNJ0lyAFsXTsjBbfaDjzALeQsN6M' crossorigin='anonymous'>" << std::endl
        << "</head>" << std::endl
        << "<body>" << std::endl;
        /*
        << "    <img src='images/" + main_obj + ".png' />" << std::endl
        */
}

bool ReportGenerator::compare_rows(StatPair row1, StatPair row2) {
    double mean1 = MorphStats::mean(row1.first, row1.second);
    double mean2 = MorphStats::mean(row2.first, row2.second);
    return mean1 < mean2;
}

void ReportGenerator::write_table(std::ostream& stream) {

    // Start the table
    stream 
        << "    <table id='morph_table' class='table table-striped table-bordered'>"
        << std::endl
        << "    <thead>" << std::endl;

    // Write the column labels for each stat
    MorphStats::write_header_row(stream);

    // Get ready for the body of the table
    stream
        << "    </thead>" << std::endl
        << "    <tbody>" << std::endl;

    // Sort the rows by average energy
    std::sort(stat_pairs.begin(), stat_pairs.end(), compare_rows);
    for (const StatPair& row : stat_pairs)
        write_row(stream, row);

    stream 
        << "    </tbody>" << std::endl
        << "    </table>" << std::endl;
}

void ReportGenerator::write_row(std::ostream& stream, const StatPair& row) {
    stream 
        << "        <tr>" << std::endl
        << "            <td rowspan='4' style='border-bottom:1px black solid'>"
        << std::endl

        // TODO: Add image
        << "IMG HERE" << std::endl
        //<< "                <img class='table_img' src='images/" + image_name + ".png' />" 
        << std::endl
        << "            </td>" << std::endl;

    // Write the data
    double mean = MorphStats::mean(row.first, row.second);
    row.first.write_row(stream);    
    row.second.write_row(stream, mean); 
        

/*
        << "<td colspan=14>" + obj_pairs[i][0].source_name + "-" + obj_pairs[i][0].target_name + "</td></tr>";
            rows += "<tr><td>" + std::to_string(obj_pairs[i][0].CFL_count) + "</td>";
            rows += "<td>" + std::to_string(obj_pairs[i][0].time_steps) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].source_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].target_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].abs_diff_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].evol_avg)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].src_tar_avg)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].max_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].weighted_total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].weighted_total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_energy)) + "</td>";
            rows += "<td></td></tr>";
            
            rows += "<tr><td colspan=14>" + obj_pairs[i][1].source_name + "-" + obj_pairs[i][1].target_name + "</td></tr>";
            rows += "<tr><td>" + std::to_string(obj_pairs[i][1].CFL_count) + "</td>";
            rows += "<td>" + std::to_string(obj_pairs[i][1].time_steps) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].source_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].target_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].abs_diff_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].evol_avg)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].src_tar_avg)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].max_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].weighted_total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].weighted_total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_energy)) + "</td>";
            rows += "<td><strong>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].mean)) + "</stong></td></tr>";
            */
}

void ReportGenerator::write_footer(std::ostream& stream) {
        stream << "    <script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.11.0/umd/popper.min.js' integrity='sha384-b/U6ypiBEHpOf/4+1nzFpr53nxSS+GLCkfwBdFNTxtclqqenISfwAzpKaMNFNmj4' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/js/bootstrap.min.js' integrity='sha384-h0AbiXch4ZDo7tp9hKZ4TsHbi047NrKGLO3SEJAg45jXxnGIfYzk4Si90RDIqNm1' crossorigin='anonymous'></script>" << std::endl
        << "    <script src='script.js'></script>" << std::endl
        << "</body>" << std::endl
        << "</html>" << std::endl;
}
