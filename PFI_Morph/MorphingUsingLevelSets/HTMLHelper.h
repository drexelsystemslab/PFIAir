#ifndef HTML_HELPER
#define HTML_HELPER

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <openvdb/openvdb.h>
#include "CommonOperations.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace HTMLHelper {
    
    class TableRow {
    public:
        TableRow() {};
        size_t CFL_count;
        int time_steps, source_surface_count, target_surface_count, abs_diff_count;
        double total_curv, weighted_total_curv, max_curv, total_val, weighted_total_val, total_energy, mean;
        double evol_avg, src_tar_avg;
        std::string source_name, target_name;
    };
    
    
    bool comparisionFunction (std::vector<HTMLHelper::TableRow> row1, std::vector<HTMLHelper::TableRow> row2) {
        return row1[1].mean < row2[1].mean;
    }
    
    std::string generateTableRowsHTML(std::vector<std::vector<HTMLHelper::TableRow>>& obj_pairs) {
        
//        std::sort(obj_pairs.begin(), obj_pairs.end(), comparisionFunction);
        
        std::string rows = "";
        std::string image_name;
        
        for(int i = 0; i < obj_pairs.size(); i++) {
            if(obj_pairs[i][0].target_name.substr(0, 8) == "mushroom") image_name = "mushroom";
            else if(obj_pairs[i][0].target_name.substr(0, 4) == "nail") image_name = "nail";
            else image_name = obj_pairs[i][0].target_name;
            
            
            
            
//            obj_pairs[i][0].weighted_total_curv = obj_pairs[i][0].total_curv / obj_pairs[i][0].evol_avg;
//            obj_pairs[i][0].weighted_total_val = (obj_pairs[i][0].total_val / obj_pairs[i][0].evol_avg) * 100;
//            obj_pairs[i][0].total_energy = obj_pairs[i][0].weighted_total_curv + obj_pairs[i][0].weighted_total_val;
//            
//            obj_pairs[i][1].weighted_total_curv = obj_pairs[i][1].total_curv / obj_pairs[i][1].evol_avg;
//            obj_pairs[i][1].weighted_total_val = (obj_pairs[i][1].total_val / obj_pairs[i][1].evol_avg) * 100;
//            obj_pairs[i][1].total_energy = obj_pairs[i][1].weighted_total_curv + obj_pairs[i][1].weighted_total_val;
//            obj_pairs[i][1].mean = (obj_pairs[i][0].total_energy + obj_pairs[i][1].total_energy) / 2;
            
            
            
            
            rows += "<tr><td rowspan='4' style='border-bottom:1px black solid'><img class='table_img' src='images/" + image_name + ".png' /></td>";
            rows += "<td colspan=14>" + obj_pairs[i][0].source_name + "-" + obj_pairs[i][0].target_name + "</td></tr>";
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
        }
        std::sort(obj_pairs.begin(), obj_pairs.end(), comparisionFunction);
        std::cout << rows << std::endl;
        return rows;
    }
    
    void writeReport(std::vector<std::vector<HTMLHelper::TableRow>>& obj_pairs, std::string main_obj) {
        
        std::ofstream file;
        
        std::string html = "", table = "";
        std::string path = "Reports/";
        
        CommonOperations::makeDirs(path.c_str());
        
        table += "<table id='morph_table' class='table table-striped table-bordered'>";
        table += "<thead><tr>";
        table += "<th>Image</th>";
        table += "<th>CFL Iterations</th>";
        table += "<th>Time steps</th>";
        table += "<th>Source voxel count</th>";
        table += "<th>Target voxel count</th>";
        table += "<th>Abs diff</th>";
        table += "<th>Evolving average</th>";
        table += "<th>Source-target average</th>";
        table += "<th>Total Curvature</th>";
        table += "<th>Max Curvature</th>";
        table += "<th>Curvature / unit area</th>";
        table += "<th>Total Value</th>";
        table += "<th>(Value / unit area) * 100</th>";
        table += "<th>Total Energy</th>";
        table += "<th>Mean</th>";
        table += "</tr></thead>";
        table += "<tbody>";
        table += generateTableRowsHTML(obj_pairs);
        table += "</tbody></table>";
        
        html += "<html>";
        html += "<head>";
        html += "<link rel='stylesheet' href='style.css'>";
        html += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css' integrity='sha384-/Y6pD6FV/Vv2HJnA6t+vslU6fwYXjCFtcEpHbNJ0lyAFsXTsjBbfaDjzALeQsN6M' crossorigin='anonymous'>";
        html += "</head>";
        html += "<body>";
        html += "<img src='images/" + main_obj + ".png' />";
        html += table;
        html += "<script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>";
        html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.11.0/umd/popper.min.js' integrity='sha384-b/U6ypiBEHpOf/4+1nzFpr53nxSS+GLCkfwBdFNTxtclqqenISfwAzpKaMNFNmj4' crossorigin='anonymous'></script>";
        html += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/js/bootstrap.min.js' integrity='sha384-h0AbiXch4ZDo7tp9hKZ4TsHbi047NrKGLO3SEJAg45jXxnGIfYzk4Si90RDIqNm1' crossorigin='anonymous'></script>";
        html += "<script src='script.js'></script>";
        html += "</body";
        html += "</html>";
        
        std::string file_path = path + main_obj + ".html";
        file.open(file_path);
        file << html;
        file.close();
    }
    
    std::vector<std::vector<HTMLHelper::TableRow>> getObjectFromState(std::string filepath) {
        std::ifstream i(filepath);
        json r_all;
        i >> r_all;
        
        std::vector<std::vector<HTMLHelper::TableRow>> all_pairs;
        std::vector<HTMLHelper::TableRow> row_pair;
        
        HTMLHelper::TableRow row1, row2;
        json temp;
        for(auto r_pair : r_all) {
            json r1 = r_pair[0];
            json r2 = r_pair[1];
            
            row1 = HTMLHelper::TableRow();
            row2 = HTMLHelper::TableRow();
            
            row_pair.push_back(row1);
            row_pair.push_back(row2);
            
            for(int i = 0; i < 2; i++) {
                if(i == 0) temp = r1;
                else temp = r2;
                
                row_pair[i].CFL_count = temp["CFL_count"].get<std::size_t>();
                row_pair[i].time_steps = temp["time_steps"].get<int>();
                row_pair[i].source_surface_count = temp["source_surface_count"].get<int>();
                row_pair[i].target_surface_count = temp["target_surface_count"].get<int>();
                row_pair[i].abs_diff_count = temp["abs_diff_count"].get<int>();
                row_pair[i].evol_avg = temp["evol_avg"].get<double>();
                row_pair[i].src_tar_avg = temp["src_tar_avg"].get<double>();
                row_pair[i].total_curv = temp["total_curv"].get<double>();
                row_pair[i].max_curv = temp["max_curv"].get<double>();
                row_pair[i].weighted_total_curv = temp["weighted_total_curv"].get<double>();
                row_pair[i].total_val = temp["total_val"].get<double>();
                row_pair[i].weighted_total_val = temp["weighted_total_val"].get<double>();
                row_pair[i].total_energy = temp["total_energy"].get<double>();
                row_pair[i].mean = temp["mean"].get<double>();
                row_pair[i].source_name = temp["source_name"].get<std::string>();
                row_pair[i].target_name = temp["target_name"].get<std::string>();
            }
             
            all_pairs.push_back(row_pair);
            row_pair.clear();
        }
        return all_pairs;
    }
    
    void saveObjectState(std::vector<std::vector<HTMLHelper::TableRow>>& pairs, std::string file_name) {
        json r, r_pair, r_all;
        for(int i = 0; i < pairs.size(); i++) {
            for(int j = 0; j < 2; j++) {
                r = {
                    {"CFL_count", pairs[i][j].CFL_count},
                    {"time_steps", pairs[i][j].time_steps},
                    {"source_surface_count", pairs[i][j].source_surface_count},
                    {"target_surface_count", pairs[i][j].target_surface_count},
                    {"abs_diff_count", pairs[i][j].abs_diff_count},
                    {"evol_avg", pairs[i][j].evol_avg},
                    {"src_tar_avg", pairs[i][j].src_tar_avg},
                    {"total_curv", pairs[i][j].total_curv},
                    {"max_curv", pairs[i][j].max_curv},
                    {"weighted_total_curv", pairs[i][j].weighted_total_curv},
                    {"total_val", pairs[i][j].total_val},
                    {"weighted_total_val", pairs[i][j].weighted_total_val},
                    {"total_energy", pairs[i][j].total_energy},
                    {"mean", pairs[i][j].mean},
                    {"source_name", pairs[i][j].source_name},
                    {"target_name", pairs[i][j].target_name}
                };
                r_pair.push_back(r);
            }
            r_all.push_back(r_pair);
            r_pair.clear();
        } 
        std::cout << r_all.dump() << std::endl << std::endl;
        
        std::string path = "JSON/";
        CommonOperations::makeDirs(path.c_str());
        
        std::string file_path = path + file_name;
        std::ofstream o(file_path);
        o << std::setw(4) << r_all << std::endl;
    }
}

#endif
