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
        
        double morphModels(HTMLHelper::TableRow& row, int norm_count=5, int opening_size=5) {
            openvdb::FloatGrid::Ptr source_grid = this->source_grid;
            openvdb::FloatGrid::Ptr target_grid = this->target_grid;
            
            // TODO: Is this redundant since this is done as a pre-processing step?
            GridOperations::performMorphologicalOpening(
                source_grid, norm_count, opening_size);
            GridOperations::performMorphologicalOpening(
                target_grid, norm_count, opening_size);
            
            // Time step
            double dt = this->dt;
            std::string file_path = this->morph_path;
            
            // Make sure the grids are both level sets before morphing.
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
            
            // Create a level set morphing object
            openvdb::util::NullInterrupter* interrupt = nullptr;
            openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
                        
            //solved artifacts issue
            ls_morph.setNormCount(norm_count);
            
            //TODO: These stats should be moved to a MorphStats struct
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
            
            // Make sure the output directory exists
            const char *path = file_path.c_str();
            CommonOperations::makeDirs(path);
            
            // Save the source and target grids
            // TODO: If the source and target files are renamed
            // frame0000.vdb and frame<n-1>.vdb and all the advectXXX.vdb
            // are renamed frameXXXX.vdb, it will be easier to view the
            // animation in Houdini (which detects such ranges of frame
            // numbers)
            file_name = file_path + "/_source.vdb";
            GridOperations::writeToFile(file_name, source_grid);
            
            file_name = file_path + "/_target.vdb";
            GridOperations::writeToFile(file_name, target_grid);
            
            // TODO: count could be renamed to frame_count
            int count = 0;
            while(true) {
                openvdb::FloatGrid::Ptr before_advect = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(source_grid);
                
                // Advect the level set and count the 
                // Courrant-Friedrrichs-Lewy iterations
                CFL_count += ls_morph.advect(time, time + time_inc);
                
                // Increment the frame number
                count++;

                // Update
                source_surface_count_sum += GridOperations::countSurfaceVoxels(source_grid);
                
                // Calculate the energy used for this frame and update stats
                // Note: source_grid has been updated after advection
                this->calculateEnergy(before_advect, source_grid, mc_sum, val_sum, max_curv);
                energy_consumed = mc_sum + val_sum;
                total_energy += energy_consumed;
                total_curv += mc_sum;
                total_val += val_sum;
                
                // TODO: Again, maybe use a numbering scheme that includes
                // source and target frames for easier viewing in Houdini.
                // TODO: Consider making these VDB files optional. They are
                // helpful for debugging and for making morph animations,
                // but not necessary for computing the energy
                file_name = file_path + "/advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
                GridOperations::writeToFile(file_name, source_grid);
                
                // Print a summary of this frame to the console
                std::cout << "CFL iterations - " << CFL_count << std::endl;
                std::cout << "File created - " << file_name << std::endl;
                std::cout << "Energy - " << (mc_sum + val_sum) << std::endl;
                 
                // Step forward in time
                time += time_inc;
                
                // TODO: a new Morph class should check these conditions
                // at the top of the loop.
                // it looks like there is a limit of 500 frames to this
                // animation. be more explicit about this with a for loop
                if((int)(time/time_inc) > 500) break;
                if(energy_consumed < 10) break;
                if(this->checkStopMorph(source_grid, target_grid)) break;
            }
            
            // Average the number of surface voxels over the maximum
            // number of frames in the animation
            double source_surface_count_avg = source_surface_count_sum / ((int)(time/time_inc) + 1);
            
            // Update the stats object
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
                // Iterate over active values on the surface only
                if(iter.getValue() < 0) {
                    if(GridOperations::checkIfSurface(iter, source_grid)) {
                        // Compare the source and target distance values at this position
                        curr_diff = openvdb::math::Abs(iter.getValue() - target_acc.getValue(iter.getCoord()));
                        if(curr_diff > max_diff) max_diff = curr_diff;
                    }
                }
            }
            std::cout << "Max diff - " << max_diff << std::endl << std::endl;
            return max_diff < threshold;
        }
        
        
        void calculateEnergy(const openvdb::FloatGrid::Ptr& grid_t1, const openvdb::FloatGrid::Ptr& grid_t2, double& mc_sum, double& val_sum, double& max_curv) {
            /**
             * Mean curvature is defined in OpenVDB as
             *
             * div((grad phi)/|grad phi|)
             *
             * alpha = grad phi
             * beta = |grad phi|
             */
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
                    
                    // Compute mean curvature at this point on the source
                    // grid
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
                    
                    // Compute mean curvature at this point on the
                    // target grid
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
