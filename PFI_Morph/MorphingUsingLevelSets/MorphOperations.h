#ifndef MORPHOPERATIONS_H
#define MORPHOPERATIONS_H

#include <openvdb/tools/LevelSetMorph.h>
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/util/NullInterrupter.h>
#include <fstream>

#include "CommonOperations.h"
#include "GridOperations.h"

namespace MorphOperations {
    
    double morphModels(openvdb::FloatGrid::Ptr&, openvdb::FloatGrid::Ptr&, double, std::string, std::string, std::string, std::string, std::string&);
    void computeMeanCurvature(const openvdb::math::Transform, const openvdb::FloatGrid::Ptr, std::map<openvdb::Coord, double>&);
    double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>&, std::map<openvdb::Coord, double>&);
    
    bool checkStopMorph(const openvdb::FloatGrid::Ptr&, const openvdb::FloatGrid::Ptr&);
    double calculateEnergy(openvdb::FloatGrid::Ptr&, openvdb::FloatGrid::Ptr&, double& mc_sum, double&);
    
    
    void computeMeanCurvature(const openvdb::FloatGrid::Ptr grid_pointer, std::map<openvdb::Coord, double>& coord_meancurv) {
        double alpha, beta;
        const openvdb::math::Transform grid_transform = grid_pointer->transform();
        //std::cout << grid_transform.baseMap()->type() << std::endl;
        
        openvdb::math::MeanCurvature<openvdb::math::MapBase, openvdb::math::DDScheme::CD_SECOND, openvdb::math::DScheme::CD_2ND> mc_obj;
        
        for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0 && GridOperations::checkIfSurface(iter, grid_pointer)) {
                mc_obj.compute(*grid_transform.baseMap(), grid_pointer->getAccessor(), iter.getCoord(), alpha, beta);
                
                if(beta != 0.0)
                    coord_meancurv[iter.getCoord()] = alpha / beta;
                else
                    std::cout << "Infinite curvature at coord " << iter.getCoord() << std::endl;
            }
        }
    } 
    
    double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>& source_coord_meancurv, std::map<openvdb::Coord, double>& target_coord_meancurv) {
        try {
            double sum_of_differences = 0.0;
            
            for (std::map<openvdb::Coord, double>::iterator it = source_coord_meancurv.begin(); it != source_coord_meancurv.end(); ++it) {
                //if terniary operator is not performed, target coords count is increased to accomodate default value
                sum_of_differences += openvdb::math::Abs((target_coord_meancurv.count(it->first) > 0 ? target_coord_meancurv[it->first] : 0) - source_coord_meancurv[it->first]);
            }
            return sum_of_differences / source_coord_meancurv.size();
        }
        catch(...) {
            std::cout << "Error" << std::endl;
            return 0;
        }
    }
    
    bool checkStopMorph(const openvdb::FloatGrid::Ptr& source_grid, const openvdb::FloatGrid::Ptr& target_grid) {
        double threshold = 0.0125, max_diff = 0.0, curr_diff;
        openvdb::Vec3d coord1, coord2;
        openvdb::FloatGrid::Accessor target_acc = target_grid->getAccessor();
        
        
        
        for (openvdb::FloatGrid::ValueOnIter iter = source_grid->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0) {
                if(GridOperations::checkIfSurface(iter, source_grid)) {
                    curr_diff = openvdb::math::Abs(iter.getValue() - target_acc.getValue(iter.getCoord()));
                    if(curr_diff > max_diff) max_diff = curr_diff;
                }
            }
        }
        std::cout << "Max diff - " << max_diff << std::endl << std::endl;
        return max_diff < threshold;
    }
    
    
    double calculateEnergy(openvdb::FloatGrid::Ptr& grid_t1, openvdb::FloatGrid::Ptr& grid_t2, double& mc_sum, double& val_sum) {
        double alpha, beta, mc_1, mc_2, abs_sum_mc_diff = 0.0, abs_sum_val_diff = 0.0, mc_const = 0.1;
        int count = 0;
        
        const openvdb::math::Transform g1_transform = grid_t1->transform(), g2_transform = grid_t2->transform();
        openvdb::math::MapBase::ConstPtr map1 = g1_transform.baseMap(), map2 = g2_transform.baseMap();
        openvdb::FloatGrid::Accessor g1_accessor = grid_t1->getAccessor(), g2_accessor = grid_t2->getAccessor();
        
        openvdb::math::MeanCurvature<openvdb::math::MapBase, openvdb::math::DDScheme::CD_SECOND, openvdb::math::DScheme::CD_2ND> mean_curv;
        
        //iterating on the source active surface voxels
        for (openvdb::FloatGrid::ValueOnIter iter = grid_t1->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0 && GridOperations::checkIfSurface(iter, grid_t1)) {
                
                count++;
                mean_curv.compute(*map1, g1_accessor, iter.getCoord(), alpha, beta);
                if(beta != 0.0)
                    mc_1 = alpha / beta;
                else {
                    mc_1 = 0.0;
                    std::cout << "Infinite curvature at source coord: " << iter.getCoord() << std::endl;
                }
                
                mean_curv.compute(*map2, g2_accessor, iter.getCoord(), alpha, beta);
                if(beta != 0.0)
                    mc_2 = alpha / beta;
                else {
                    mc_2 = 0.0;
                    std::cout << "Infinite curvature at target coord: " << iter.getCoord() << std::endl;
                }
                
                abs_sum_mc_diff += mc_const * openvdb::math::Abs(mc_1 - mc_2);
                abs_sum_val_diff += openvdb::math::Abs(iter.getValue() - g2_accessor.getValue(iter.getCoord()));
            }
        }
        
        std::ofstream file;
        file.open ("log.txt", std::ios_base::app);
        file << "MC Sum: " << abs_sum_mc_diff << "\t\tValue Sum: " << abs_sum_val_diff << "\t\tTotal: " << abs_sum_mc_diff + abs_sum_val_diff << "\n";
        file.close();
        
        mc_sum = abs_sum_mc_diff;
        val_sum = abs_sum_val_diff;
        return abs_sum_mc_diff + abs_sum_val_diff;
    }
    
    double morphModels(openvdb::FloatGrid::Ptr& source_grid, openvdb::FloatGrid::Ptr& target_grid, double dt, std::string title, std::string dir, std::string voxel_size, std::string dt_str, std::string& table) {
        //adjustScale(source_grid, target_grid);
        
        openvdb::util::NullInterrupter* interrupt = nullptr;
        openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
        
        size_t CFL_count = 0.0;
        double time = 0.0, time_inc = dt, energy_consumed = 0.0, total_energy = 0.0, total_curv = 0.0, total_val = 0.0;
        double mc_sum = 0.0, val_sum = 0.0;
        std::string file_name = "";
        
        std::string str_path = "output/r1sx/dt" + dt_str + "/" + voxel_size + "/" + dir + "";
        const char *path = str_path.c_str();
        CommonOperations::makeDirs(path);
        
        file_name = "output/r1sx/dt" + dt_str + "/" + voxel_size + "/" + dir + "/_init.vdb";
        GridOperations::writeToFile(file_name, source_grid);
        
        while(true) {
            openvdb::FloatGrid::Ptr before_advect = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(source_grid);
            
            CFL_count += ls_morph.advect(time, time + time_inc);
            
            //source_grid has been updated after advection
            energy_consumed = calculateEnergy(before_advect, source_grid, mc_sum, val_sum);
            total_energy += energy_consumed;
            total_curv += mc_sum;
            total_val += val_sum;
            
            file_name = "output/r1sx/dt" + dt_str + "/" + voxel_size + "/" + dir + "/advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
            GridOperations::writeToFile(file_name, source_grid);
            
            CommonOperations::display("CFL iterations", CFL_count);
            CommonOperations::display("File created", file_name);
            CommonOperations::display("Energy", energy_consumed);
            CommonOperations::display("Total", total_energy);
            
            time += time_inc;
            
            if(checkStopMorph(source_grid, target_grid)) break;
        }
        
        //    std::ofstream file;
        //    file.open ("log.txt", std::ios_base::app);
        //    file << title + "\n";
        //    file << "Iterations - " << CFL_count << "\n";
        //    file << "Total energy consumed - " << total_energy << "\n";
        //    file << "\n";
        //    file.close();
        
        table += "<tr><td>" + std::to_string(CFL_count) + "</td>";
        table += "<td>" + std::to_string((int)(time/time_inc)) + "</td>";
        table += "<td>" + std::to_string(total_curv) + "</td>";
        table += "<td>" + std::to_string(total_val) + "</td>";
        table += "<td>" + std::to_string(total_energy) + "</td>";
        
        return total_energy;
    }
}

#endif