#include "Morph.h"

MorphStats Morph::morph(
        const LevelSet& source, 
        const LevelSet& target,
        std::string frames_dir) {
    // Check if we should save frames
    bool save_frames = frames_dir != "";

    // Set up a new MorphStats
    stats = MorphStats();

    // Compute stats about source and target level sets
    double source_count = source.count_surface_voxels();
    double target_count = target.count_surface_voxels(); 
    stats.count_surface_voxels(source_count, target_count);

    // We don't want to modify the input grids, so deep copy
    // the first frame.
    LevelSet current_ls = source.deep_copy();

    // Initialize the OpenVDB morph object
    init_morph(current_ls.get_level_set(), target.get_level_set());

    // Save the first frame frame
    if (save_frames) {
        std::cout << "Saving first frame: " << frame_fname(frames_dir, 0)
            << std::endl;
    }

    // Energy consumed for this frame. This is used for the stopping
    // condition. Set this above the threshold for the first frame.
    double energy_consumed = MIN_ENERGY + 1;

    // Declare frame_count outside the scope of the loop, we will need
    // the final value to save the last frame.
    int frame_count;
    for (frame_count = 1; frame_count < MAX_ITERS; frame_count++) {
        // Check if we've consumed too much energy
        if (energy_consumed < MIN_ENERGY)
            break;

        // Check if the morph is finished
        if (morph_is_finished(current_ls, target))
            break;

        //if(this->checkStopMorph(source_grid, target_grid)) break;

        // Start and end times for this advection step
        double start_time = frame_count * TIME_STEP;
        double end_time = start_time + TIME_STEP;
        stats.increment_time();

        // We need to compute the energy between each pair of frames.
        // Thus we must make a copy of the 
        LevelSet prev_ls = current_ls.deep_copy();

        // Advect the level set and count the 
        // Courrant-Friedrrichs-Lewy iterations
        // NOTE: This modifies current_ls in place
        double cfl_iters = ls_morph->advect(start_time, end_time);
        stats.add_cfl_iterations(cfl_iters);

        // Update voxel count from the new frame
        double surface_voxel_count = current_ls.count_surface_voxels();
        stats.add_surface_voxels(surface_voxel_count);

        // Update the energy calculation
        EnergyResults energy = calculate_energy(prev_ls, current_ls);
        stats.update_energy(energy.delta_curvature, energy.delta_curvature);
        stats.update_max_curvature(energy.max_curvature);  

        std::cout << "\nFrame " << frame_count << " -----------" << std::endl;
        std::cout << "CFL iterations - " << cfl_iters << std::endl;
        std::cout << "(dCurvature, dValue, max_Curvature) = ("
            << energy.delta_curvature << ", " << energy.delta_value << ", "
            << energy.max_curvature << ")" << std::endl;

        // Optionally save the frame
        if (save_frames) { 
            std::cout << "Saving frame " 
                << frame_fname(frames_dir, frame_count) << std::endl;
        }
    }

    if (save_frames) {
        std::cout << "\nSaving last frame: " 
            << frame_fname(frames_dir, frame_count) << std::endl;
    }

    stats.finalize_stats();
    return stats;
}

void Morph::init_morph(
        GridType::Ptr source_grid, GridType::ConstPtr target_grid) {
    // Create a level set morphing object
    InterruptType* interrupt = nullptr;
    ls_morph = std::unique_ptr<LSMorph>(
        new LSMorph(*source_grid, *target_grid, interrupt));
    ls_morph->setNormCount(NORM_COUNT);
}

bool Morph::morph_is_finished(
        const LevelSet& current, const LevelSet& target) {
    return false;
}

EnergyResults Morph::calculate_energy(
        const LevelSet& prev, const LevelSet& next) {
    EnergyResults results;
    results.delta_curvature = 0.0;
    results.delta_value = 0.0;
    results.max_curvature = 0.0;
}

std::string Morph::frame_fname(std::string frames_dir, int frame) { 
    std::string frame_str = std::to_string(frame);
    return frames_dir + "frame_" + frame_str + ".vdb";
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
