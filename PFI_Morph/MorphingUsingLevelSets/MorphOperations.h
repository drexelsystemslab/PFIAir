#ifndef MORPHOPERATIONS_H
#define MORPHOPERATIONS_H

#include <openvdb/tools/LevelSetMorph.h>
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/util/NullInterrupter.h>
#include <fstream>

#include "CommonOperations.h"
#include "GridOperations.h"
#include "HTMLHelper.h"

namespace MorphOperations {
    class Morph {
    public:
        
        std::string file_name, morph_path,
        curr_obj, curr_name, curr_name_wo_ext, curr_path, obj_path, vdb_path;
        
        int source_nb = 3, target_nb = 30;
        double voxel_size = 0.01, dt = 0.25;
        
        std::string ignore_arr[40] = {
            "",
            "beast.stl.obj",
            "bird.stl.obj",
//            "claw.stl.obj", 
//            "container.stl.obj",
            //"cylinder.stl.obj",
            "duck.stl.obj",
            "flower.stl.obj",
            "fork.stl.obj",
//            "hand.stl.obj",
            "iron-board.stl.obj",
            "man-gun.stl.obj",
            "mask.stl.obj",
            "modular-hand.stl.obj",
//            "mushroom-1.stl.obj",
            "mushroom-2.stl.obj",
            "mushroom-3.stl.obj",
            "mushroom-4.stl.obj",
            "mushroom-5.stl.obj",
//            "nail-1.stl.obj",
            "nail-2.stl.obj",
            "nail-3.stl.obj",
//            "pebble.stl.obj",
//            "pendulum.stl.obj",
//            "spaceship.stl.obj",
            "spoon.stl.obj",
            "spring.stl.obj",
            "stand.stl.obj",
            "swan.stl.obj",
//            "tool.stl.obj",
            "tusk.stl.obj",
            "vase.stl.obj",
//            "winding-wheel.stl.obj",
            //bishop.stl.obj,
            //cup.stl.obj,
            //flask.stl.obj,
            //hammer.stl.obj,
            //head.stl.obj,
            //queen.stl.obj,
            //table.stl.obj,
            ".DS_Store"
        };
        
        Morph(std::string curr_obj, std::string curr_name, std::string curr_name_wo_ext, std::string curr_path, std::string obj_path, std::string vdb_path) {
            this->curr_obj = curr_obj;
            this->curr_name = curr_name;
            this->curr_name_wo_ext = curr_name_wo_ext;
            this->curr_path = curr_path;
            this->obj_path = obj_path;
            this->vdb_path = vdb_path;
            this->ignore_arr[0] = curr_obj;
        }
        
        Morph(std::string curr_obj) {
            this->curr_obj = curr_obj;
            this->ignore_arr[0] = curr_obj;
        }
        
        openvdb::FloatGrid::Ptr source_grid;
        openvdb::FloatGrid::Ptr target_grid;
        
        double morphModels(HTMLHelper::TableRow& row) {
            openvdb::FloatGrid::Ptr source_grid = this->source_grid;
            openvdb::FloatGrid::Ptr target_grid = this->target_grid;
            
            GridOperations::performMorphologicalOpening(source_grid);
            GridOperations::performMorphologicalOpening(target_grid);
            
            double dt = this->dt;
            std::string file_path = this->morph_path;
            
            //getGridClass == 1 -> openvdb::GRID_LEVEL_SET
            if(source_grid->getGridClass() != 1 && target_grid->getGridClass() != 1) {
                std::cout<< "Grids are not level sets";
                return 0;
            }
            
            // Get stats about the source/target voxel counts
            row.source_surface_count = GridOperations::countSurfaceVoxels(source_grid);
            row.target_surface_count = GridOperations::countSurfaceVoxels(target_grid);
            row.abs_diff_count = openvdb::math::Abs(row.source_surface_count - row.target_surface_count);
            row.src_tar_avg = (row.source_surface_count + row.target_surface_count) / 2;
            
            openvdb::util::NullInterrupter* interrupt = nullptr;
            openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
                        
            //solved artifacts issue
            ls_morph.setNormCount(5);
            
            size_t CFL_count = 0.0;
            double time = 0.0;
            double time_inc = dt;
            double energy_consumed = 0.0;
            double total_energy = 0.0;
            double total_curv = 0.0; 
            double total_val = 0.0;
            double mc_sum = 0.0;
            double val_sum = 0.0;
            double weight_val = 100;
            double max_curv = 0.0;
            double source_surface_count_sum = GridOperations::countSurfaceVoxels(source_grid);
            
            std::string file_name = "";
            
            const char *path = file_path.c_str();
            CommonOperations::makeDirs(path);
            
            file_name = file_path + "/_source.vdb";
            GridOperations::writeToFile(file_name, source_grid);
            
            file_name = file_path + "/_target.vdb";
            GridOperations::writeToFile(file_name, target_grid);
            
            int count = 0;
            while(true) {
                openvdb::FloatGrid::Ptr before_advect = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(source_grid);
                
                // Advect the level set and count the 
                // Courrant-Friedrrichs-Lewy iterations
                CFL_count += ls_morph.advect(time, time + time_inc);
                
                count++;
                source_surface_count_sum += GridOperations::countSurfaceVoxels(source_grid);
                
                //source_grid has been updated after advection
                this->calculateEnergy(before_advect, source_grid, mc_sum, val_sum, max_curv);
                energy_consumed = mc_sum + val_sum;
                total_energy += energy_consumed;
                total_curv += mc_sum;
                total_val += val_sum;
                
                file_name = file_path + "/advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
                GridOperations::writeToFile(file_name, source_grid);
                
                std::cout << "CFL iterations - " << CFL_count << std::endl;
                std::cout << "File created - " << file_name << std::endl;
                std::cout << "Energy - " << (mc_sum + val_sum) << std::endl;
                
                
                time += time_inc;
                
                if((int)(time/time_inc) > 500) break;
                if(energy_consumed < 10) break;
                if(this->checkStopMorph(source_grid, target_grid)) break;
            }
            
            double source_surface_count_avg = source_surface_count_sum / ((int)(time/time_inc) + 1);
            
            row.CFL_count = CFL_count;
            row.time_steps = (int)(time/time_inc);
            row.total_curv = total_curv;
            row.weighted_total_curv = total_curv / source_surface_count_avg;
            row.total_val = total_val;
            row.weighted_total_val = (total_val / source_surface_count_avg) * weight_val;
            row.total_energy = row.weighted_total_curv + row.weighted_total_val;
            row.evol_avg = source_surface_count_avg;
            row.max_curv = max_curv;
            return row.total_energy;
        }
        
        bool checkStopMorph(const openvdb::FloatGrid::Ptr& source_grid, const openvdb::FloatGrid::Ptr& target_grid) {
            double threshold = target_grid->voxelSize()[0], max_diff = 0.0, curr_diff;
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
        
        
        void calculateEnergy(const openvdb::FloatGrid::Ptr& grid_t1, const openvdb::FloatGrid::Ptr& grid_t2, double& mc_sum, double& val_sum, double& max_curv) {
            double alpha, beta, mc_1, mc_2, abs_sum_mc_diff = 0.0, voxel_size = grid_t1->voxelSize()[0];
            int count = 0;
            int coord_diff_count = 0;
            double max_diff = 0, curr_val;
            
            const openvdb::math::Transform g1_transform = grid_t1->transform(), g2_transform = grid_t2->transform();
            openvdb::math::MapBase::ConstPtr map1 = g1_transform.baseMap(), map2 = g2_transform.baseMap();
            openvdb::FloatGrid::Accessor g1_accessor = grid_t1->getAccessor(), g2_accessor = grid_t2->getAccessor();
            
            openvdb::math::MeanCurvature<openvdb::math::MapBase, openvdb::math::DDScheme::CD_SECOND, openvdb::math::DScheme::CD_2ND> mean_curv;
            
            //iterating on the source active surface voxels
            for (openvdb::FloatGrid::ValueOnIter iter = grid_t1->beginValueOn(); iter; ++iter) {
                if(iter.getValue() < 0 && GridOperations::checkIfSurface(iter, grid_t1)) {
                    
                    count++;
                    mean_curv.compute(*map1, g1_accessor, iter.getCoord(), alpha, beta);
                    if(beta != 0.0) {
                        mc_1 = alpha / beta;
                        if(mc_1 > max_curv) max_curv = mc_1;
                    }
                    else {
                        mc_1 = 0.0;
                        max_curv = std::numeric_limits<int>::max();
                        std::cout << "Infinite curvature at source coord: " << iter.getCoord() << std::endl;
                    }
                    
                    mean_curv.compute(*map2, g2_accessor, iter.getCoord(), alpha, beta);
                    if(beta != 0.0) {
                        mc_2 = alpha / beta;
                        if(mc_2 > max_curv) max_curv = mc_2;
                    }
                    else {
                        mc_2 = 0.0;
                        max_curv = std::numeric_limits<int>::max();
                        std::cout << "Infinite curvature at target coord: " << iter.getCoord() << std::endl;
                    }
                    
                    abs_sum_mc_diff += openvdb::math::Abs(mc_1 - mc_2);
                    curr_val = openvdb::math::Abs(iter.getValue() - g2_accessor.getValue(iter.getCoord()));
                    if(curr_val > max_diff) max_diff = curr_val;
                    
                    if(openvdb::math::Abs(iter.getValue() - g2_accessor.getValue(iter.getCoord())) > voxel_size)
                        val_sum += 1;
                }
            }
            std::cout << max_diff << std::endl;
            mc_sum = abs_sum_mc_diff;
        }
    };
    
    
    
    
   }

#endif
