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
        int time_steps, source_surface_count, target_surface_count;
        double total_curv, total_val, total_energy, mean;
        std::string source_name, target_name;
    };
    
    
    bool comparisionFunction (std::vector<HTMLHelper::TableRow> row1, std::vector<HTMLHelper::TableRow> row2) {
        return row1[1].mean < row2[1].mean;
    }
    
    std::string generateTableRowsHTML(std::vector<std::vector<HTMLHelper::TableRow>>& obj_pairs) {
        
        std::sort(obj_pairs.begin(), obj_pairs.end(), comparisionFunction);
        
        std::string rows = "";
        std::string image_name;
        
        for(int i = 0; i < obj_pairs.size(); i++) {
            if(obj_pairs[i][0].target_name.substr(0, 8) == "mushroom") image_name = "mushroom";
            else if(obj_pairs[i][0].target_name.substr(0, 4) == "nail") image_name = "nail";
            else image_name = obj_pairs[i][0].target_name;
            
            rows += "<tr><td colspan='3'><img src='images/" + image_name + ".png' /></td><td colspan='6'></td></tr>";
            
            rows += "<tr><td colspan=9>" + obj_pairs[i][0].source_name + "-" + obj_pairs[i][0].target_name + "</td></tr>";
            rows += "<tr><td>" + std::to_string(obj_pairs[i][0].CFL_count) + "</td>";
            rows += "<td>" + std::to_string(obj_pairs[i][0].time_steps) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].source_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].target_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(openvdb::math::Abs(obj_pairs[i][0].source_surface_count - obj_pairs[i][0].target_surface_count))) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_energy)) + "</td>";
            rows += "<td></td></tr>";
            
            rows += "<tr><td colspan=9>" + obj_pairs[i][1].source_name + "-" + obj_pairs[i][1].target_name + "</td></tr>";
            rows += "<tr><td>" + std::to_string(obj_pairs[i][1].CFL_count) + "</td>";
            rows += "<td>" + std::to_string(obj_pairs[i][1].time_steps) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].source_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].target_surface_count)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(openvdb::math::Abs(obj_pairs[i][1].source_surface_count - obj_pairs[i][1].target_surface_count))) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_curv)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_val)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_energy)) + "</td>";
            rows += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].mean)) + "</td></tr>";
        }
        
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
        table += "<th>CFL Iterations</th>";
        table += "<th>Time steps</th>";
        table += "<th>Source voxel count</th>";
        table += "<th>Target voxel count</th>";
        table += "<th>Abs diff</th>";
        table += "<th>Total Curvature</th>";
        table += "<th>Total Value</th>";
        table += "<th>Total Energy</th>";
        table += "<th>Mean</th>";
        table += "</tr></thead>";
        table += "<tbody>";
        table += generateTableRowsHTML(obj_pairs);
        table += "</tbody></table>";
        
        html += "<html>";
        html += "<head>";
        html += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css' integrity='sha384-/Y6pD6FV/Vv2HJnA6t+vslU6fwYXjCFtcEpHbNJ0lyAFsXTsjBbfaDjzALeQsN6M' crossorigin='anonymous'>";
        html+= "<style> img { width:200px; height:100px; } </style>";
        html += "</head>";
        html += "<body>";
        html += "<img src='images/" + main_obj + ".png' />";
        html += table;
        html += "<script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>";
        html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.11.0/umd/popper.min.js' integrity='sha384-b/U6ypiBEHpOf/4+1nzFpr53nxSS+GLCkfwBdFNTxtclqqenISfwAzpKaMNFNmj4' crossorigin='anonymous'></script>";
        html += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/js/bootstrap.min.js' integrity='sha384-h0AbiXch4ZDo7tp9hKZ4TsHbi047NrKGLO3SEJAg45jXxnGIfYzk4Si90RDIqNm1' crossorigin='anonymous'></script>";
        
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
        
        for(auto r_pair : r_all) {
            json r1 = r_pair[0];
            json r2 = r_pair[1];
            
            row1 = HTMLHelper::TableRow();
            row2 = HTMLHelper::TableRow();
            
            row1.CFL_count = r1["CFL_count"].get<std::size_t>();
            row1.time_steps = r1["time_steps"].get<int>();
            row1.source_surface_count = r1["source_surface_count"].get<int>();
            row1.target_surface_count = r1["target_surface_count"].get<int>();
            row1.total_curv = r1["total_curv"].get<int>();
            row1.total_val = r1["total_val"].get<int>();
            row1.total_energy = r1["total_energy"].get<int>();
            row1.mean = r1["mean"].get<double>();
            row1.source_name = r1["source_name"].get<std::string>();
            row1.target_name = r1["target_name"].get<std::string>();
            
            row2.CFL_count = r2["CFL_count"].get<std::size_t>();
            row2.time_steps = r2["time_steps"].get<int>();
            row2.source_surface_count = r2["source_surface_count"].get<int>();
            row2.target_surface_count = r2["target_surface_count"].get<int>();
            row2.total_curv = r2["total_curv"].get<int>();
            row2.total_val = r2["total_val"].get<int>();
            row2.total_energy = r2["total_energy"].get<int>();
            row2.mean = r2["mean"].get<double>();
            row2.source_name = r2["source_name"].get<std::string>();
            row2.target_name = r2["target_name"].get<std::string>();
            
            row_pair.push_back(row1);
            row_pair.push_back(row2);
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
                    {"total_curv", pairs[i][j].total_curv},
                    {"total_val", pairs[i][j].total_val},
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
        std::ofstream o(file_name);
        o << std::setw(4) << r_all << std::endl;
    }
}

#endif
