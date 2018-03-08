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
            "claw.stl.obj",
            "container.stl.obj",
            "cylinder.stl.obj",
            "duck.stl.obj",
            "flower.stl.obj",
            "fork.stl.obj",
            "hand.stl.obj",
            "iron-board.stl.obj",
            "man-gun.stl.obj",
            "mask.stl.obj",
            "modular-hand.stl.obj",
            "mushroom-1.stl.obj",
            //"mushroom-2.stl.obj",
            "mushroom-3.stl.obj",
            "mushroom-4.stl.obj",
            "mushroom-5.stl.obj",
            //"nail-1.stl.obj",
            "nail-2.stl.obj",
            "nail-3.stl.obj",
            //"pebble.stl.obj",
            "pendulum.stl.obj",
            "spaceship.stl.obj",
            "spoon.stl.obj",
            "spring.stl.obj",
            "stand.stl.obj",
            "swan.stl.obj",
            "tool.stl.obj",
            "tusk.stl.obj",
            "vase.stl.obj",
            //"winding-wheel.stl.obj",
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
            double dt = this->dt;
            std::string file_path = this->morph_path;
            
            //getGridClass == 1 -> openvdb::GRID_LEVEL_SET
            if(source_grid->getGridClass() != 1 && target_grid->getGridClass() != 1) {
                std::cout<< "Grids are not level sets";
                return 0;
            }
            
            row.source_surface_count = GridOperations::countSurfaceVoxels(source_grid);
            row.target_surface_count = GridOperations::countSurfaceVoxels(target_grid);
            
            openvdb::util::NullInterrupter* interrupt = nullptr;
            openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
            
            //solved artifacts issue
            ls_morph.setNormCount(5);
            
            size_t CFL_count = 0.0;
            double time = 0.0, time_inc = dt, energy_consumed = 0.0, total_energy = 0.0, total_curv = 0.0, total_val = 0.0;
            double mc_sum = 0.0, val_sum = 0.0, weight_val = 20.0;
            
            std::string file_name = "";
            
            const char *path = file_path.c_str();
            CommonOperations::makeDirs(path);
            
            file_name = file_path + "/_source.vdb";
            GridOperations::writeToFile(file_name, source_grid);
            
            file_name = file_path + "/_target.vdb";
            GridOperations::writeToFile(file_name, target_grid);
            
            
            while(true) {
                openvdb::FloatGrid::Ptr before_advect = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(source_grid);
                
                CFL_count += ls_morph.advect(time, time + time_inc);
                
                //source_grid has been updated after advection
                this->calculateEnergy(before_advect, source_grid, mc_sum, val_sum);
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
                
                if(energy_consumed < 10) break;
                if(this->checkStopMorph(source_grid, target_grid)) break;
            }
            
            row.CFL_count = CFL_count;
            row.time_steps = (int)(time/time_inc);
            row.total_curv = (int)(total_curv /weight_val);
            row.total_val = total_val;
            row.total_energy = row.total_curv + row.total_val;
            
            return row.total_energy;
        }
        
        bool checkStopMorph(const openvdb::FloatGrid::Ptr& source_grid, const openvdb::FloatGrid::Ptr& target_grid) {
            double threshold = target_grid->voxelSize()[0] / 2, max_diff = 0.0, curr_diff;
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
        
        
        void calculateEnergy(const openvdb::FloatGrid::Ptr& grid_t1, const openvdb::FloatGrid::Ptr& grid_t2, double& mc_sum, double& val_sum) {
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
            
            mc_sum = abs_sum_mc_diff;
            val_sum = abs_sum_val_diff;
            
//            std::ofstream file;
//            file.open ("log.txt", std::ios_base::app);
//            file << "MC Sum: " << abs_sum_mc_diff << "\t\tValue Sum: " << abs_sum_val_diff << "\t\tTotal: " << abs_sum_mc_diff + abs_sum_val_diff << "\n";
//            file.close();
        }
    };
    
    
    
    
   }

#endif