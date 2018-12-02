#include "Morph.h"

MorphStats Morph::morph(
        LevelSet& source, LevelSet& target, std::string frames_dir) {

    // Check if we should save frames
    bool save_frames = frames_dir != "";

    // Set up a new MorphStats
    stats = MorphStats();
    stats.set_names(source.get_name(), target.get_name());

    // Compute stats about source and target level sets
    double source_count = source.count_surface_voxels();
    double target_count = target.count_surface_voxels(); 
    stats.count_surface_voxels(source_count, target_count);

    // Initialize the OpenVDB morph object
    init_morph(source.get_level_set(), target.get_level_set());

    int frame_count = 0;

    // Save first frame
    if (save_frames) {
        std::cout << frame_count << std::endl;
        std::cout << frames_dir << std::endl;
    }

    // Energy consumed for this frame. This is used for the stopping
    // condition. Set this to a large value to start
    double energy_consumed = MIN_ENERGY + 1;
    for (frame_count = 0; frame_count < MAX_ITERS; frame_count++) {
        if (energy_consumed < MIN_ENERGY)
            break;

        /*
        if (morph_finished(current_grid, target_grid))
            break;
        */
        //if(this->checkStopMorph(source_grid, target_grid)) break;

        // Start and end times for this advection step
        double start_time = frame_count * TIME_STEP;
        double end_time = start_time + TIME_STEP;
        stats.increment_time();

        /*
                openvdb::FloatGrid::Ptr before_advect = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(source_grid);
                
        // Advect the level set and count the 
        // Courrant-Friedrrichs-Lewy iterations
        cfl_count += ls_morph.advect(start_time, end_time);
        forwards.add_cfl_iterations(cfl_count);

        std::cout << "CFL iterations - " << CFL_count << std::endl;
                

                // Update
                source_surface_count_sum += GridOperations::countSurfaceVoxels(source_grid);
                
                // Calculate the energy used for this frame and update stats
                // Note: source_grid has been updated after advection
                this->calculateEnergy(before_advect, source_grid, mc_sum, val_sum, max_curv);

        stats.update_energy(delta_curvature, delta_value);

        forwards.update_max_curvature(max_curvature);


        std::string frame_fname = 
                
                // TODO: Again, maybe use a numbering scheme that includes
                // source and target frames for easier viewing in Houdini.
                // TODO: Consider making these VDB files optional. They are
                // helpful for debugging and for making morph animations,
                // but not necessary for computing the energy
                file_name = file_path + "/advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
                GridOperations::writeToFile(file_name, source_grid);
                
                // Print a summary of this frame to the console
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
    */
    }

    if (save_frames) {
        std::cout << frame_count << std::endl;
        std::cout << frames_dir << std::endl;
    }

    stats.finalize_stats();
    return stats;
}

void Morph::init_morph(GridType::Ptr source_grid, GridType::Ptr target_grid) {
    // Create a level set morphing object
    InterruptType* interrupt = nullptr;
    ls_morph = std::unique_ptr<LSMorph>(
        new LSMorph(*source_grid, *target_grid, interrupt));
    ls_morph->setNormCount(NORM_COUNT);
}


/*
        double morphModels(HTMLHelper::TableRow& row) {
            openvdb::FloatGrid::Ptr source_grid = this->source_grid;
            openvdb::FloatGrid::Ptr target_grid = this->target_grid;
            
            GridOperations::performMorphologicalOpening(source_grid);
            GridOperations::performMorphologicalOpening(target_grid);
            
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
            ls_morph.setNormCount(5);
            
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
*/
