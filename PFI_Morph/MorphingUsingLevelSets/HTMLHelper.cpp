#ifndef HTML_HELPER
#define HTML_HELPER

#include <string>
#include <vector>
#include <openvdb/openvdb.h>
#include "CommonOperations.h"

namespace HTMLHelper {
    
    class TableRow {
    public:
        TableRow() {};
        int CFL_count, time_steps;
        double total_curv, total_val, total_energy, mean;
        std::string source_name, target_name;
    };
    
    void createTableRow(std::vector<TableRow[2]>& obj_pairs) {
        std::string table = "";
        
        for(int i = 0; i < obj_pairs.size(); i++) {
            table += "<tr><td colspan='3'><img src='" + obj_pairs[i][0].target_name + ".png' /></td><td colspan='3'></td></tr>";
            
            table += "<tr><td colspan=6>" + obj_pairs[i][0].source_name + "-" + obj_pairs[i][0].target_name + "</td></tr>";
            table += "<tr><td>" + std::to_string(obj_pairs[i][0].CFL_count) + "</td>";
            table += "<td>" + std::to_string(obj_pairs[i][0].time_steps) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_curv)) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_val)) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][0].total_energy)) + "</td>";
            
            table += "<tr><td colspan=6>" + obj_pairs[i][1].source_name + "-" + obj_pairs[i][1].target_name + "</td></tr>";
            table += "<tr><td>" + std::to_string(obj_pairs[i][1].CFL_count) + "</td>";
            table += "<td>" + std::to_string(obj_pairs[i][1].time_steps) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_curv)) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_val)) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].total_energy)) + "</td>";
            table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(obj_pairs[i][1].mean)) + "</td></tr>";
        }
        
        std::cout << "create table row table" << std::endl;
        std::cout << table << std::endl;
    }
}

#endif
