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

    // Save the first frame frame if the directory was specified
    maybe_save_frame(frames_dir, current_ls, 0);

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

        // Output summary of frame
        std::cout << "\nFrame " << frame_count << " -----------" << std::endl;
        std::cout << "CFL iterations - " << cfl_iters << std::endl;
        std::cout << "(dCurvature, dValue, max_Curvature) = ("
            << energy.delta_curvature << ", " << energy.delta_value << ", "
            << energy.max_curvature << ")" << std::endl;

        // Optionally save the frame
        maybe_save_frame(frames_dir, current_ls, frame_count);
    }

    maybe_save_frame(frames_dir, target, frame_count);

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

    // Get the underlying grids
    openvdb::FloatGrid::ConstPtr current_grid = current.get_level_set();
    openvdb::FloatGrid::ConstPtr target_grid = target.get_level_set();

    double threshold = target.get_voxel_size();
    double max_diff = 0.0;
    double curr_diff = 0.0;

    openvdb::FloatGrid::ConstAccessor target_acc = target_grid->getAccessor();

    typedef openvdb::FloatGrid::ValueOnCIter IterType;
    for (IterType iter = current_grid->cbeginValueOn(); iter; ++iter) {
        bool is_surface = current.is_surface_voxel(iter.getCoord());
        if (iter.getValue() < 0 && is_surface) {
            // compare the voxels at the same location in the current
            // and target grids and get the absolute difference in values
            curr_diff = openvdb::math::Abs(
                iter.getValue() - target_acc.getValue(iter.getCoord()));

            // Update the maximum difference
            if (curr_diff > max_diff)
                max_diff = curr_diff;
        }
    }

    // Threshold the result
    std::cout << "Max diff - " << max_diff << std::endl;
    std::cout << "Threshold - " << threshold << std::endl;
    return max_diff < threshold;
}

EnergyResults Morph::calculate_energy(
        const LevelSet& prev, const LevelSet& curr) {

    EnergyCalculator calc(prev, curr);
    return calc.compute_energy();

    /**
     * Pseudocode for updated energy calculation algorithm
     *
     * (maximum absolute mean cuvature for this pair of frames)
     * max_curvature = 0.0
     * delta_curvature = 0.0
     * delta_value = 0.0

     * For each active surface voxel in the previous frame:
     * - compute the mean curvature of prev @ current voxel 
     * - compute the mean curvature of curr @ current voxel 
     * - if both prev/curr mean curvature are infinite:
     *   - skip to the next voxel, we cannot get any meaningful info
     * - if one of prev/curr mean curvature is finite:
     *   - max_curvature = max(max_curvature, abs(finite mean curvature))
     *   - curv_diff = |finite mean curvature|
     *   - delta_curvature += curv_diff
     * - if both prev/curr mean curvature are finite:
     *   - max_curvature = max(
     *         max_curvature, abs(prev mean curvature), abs(curr mean curvature))
     *   - curv_diff = |prev mean curvature - curr mean curvature|
     *   - delta_curvature += curv_diff
     * - delta_value = |prev.value[coord] - curr.value[coord]|
     *   
     * Return the following:
     *
     * results.delta_value: 
     * results.delta_curvature: the change in mean curvature during the morph
     *      from prev -> current
     * results.max_curvature: maximum mean curvature *for this pair of frames*
     *
     *
     * Changes to add with Curvature bounds:
     * - Precompute the maximum (and miniumum for negative curvature?) 
     *     mean curvatures for source and target models.
     * - lower_bound = min(source_min_mean_curv, target_min_mean_curv)
     * - upper_bound = max(source_max_mean_curv, target_max_mean_curv)
     * - Replace checks for "is finite" with 
     *    "is in range [lower_bound, upper_bound]"
     *
     * These bounds will make it easier to filter out extraneous values,
     * at a cost of running another pass over the voxels of source and
     * target models (once per morph)
     */

    /**
     * Mean curvature is defined in OpenVDB as
     *
     * div((grad phi)/|grad phi|)
     *
     * alpha = grad phi
     * beta = |grad phi|
     */

     /*

    double mean_curv_prev;
    double mean_curv_curr;
    double abs_diff_curv = 0.0;
    double voxel_size = prev.get_voxel_size();
    int count = 0;
    int diff_count = 0;
    double max_diff = 0.0;
    double curr_val;
    */

    // Lots of intermediate OpenVDB objects ===========================
    /*
    using namespace openvdb::math;
    openvdb::FloatGrid::ConstPtr prev_grid = prev.get_level_set();
    openvdb::FloatGrid::ConstPtr curr_grid = curr.get_level_set();

    const Transform prev_xform = prev_grid->transform();
    const Transform curr_xform = next_grid->transform();

    MapBase::ConstPtr prev_map = prev_xform.baseMap();
    MapBase::ConstPtr curr_map = curr_xform.baseMap();

    openvdb::FloatGrid::ConstAccessor prev_acc = prev_grid->getAccessor();
    openvdb::FloatGrid::ConstAccessor curr_acc = curr_grid->getAccessor();

    MeanCurvature<MapBase, DDScheme::CD_SECOND, DScheme::CD_SND> mean_curv;
    */
    
    // =================================================================


    // Iterate over surface voxels
    /*
    typedef openvdb::FloatGrid::ValueOnCIter IterType;
    for (IterType iter = prev_grid.cbeginValueOn(); iter; ++iter) {
        if (iter.get_value() < 0 && prev.is_surface_voxel(iter.getCoord()) {
            // count up surface voxels
            count++;

            // Compute the mean curvature at this point in both models ======
            double alpha;
            double beta;
            double prev_mean_curv;

            mean_curv.compute(
                *prev_map, prev_acc, iter.getCoord(), alpha, beta);

            // Check for infinite curvature
            if (beta == 0.0) {
                prev_mean_curv = std::numeric_limits<double>::infinity(); 
            }
            
            

            mean_curv.compute(
                *curr_map, curr_acc, iter.getCoor(), alpha, beta);

            if (beta != 0.0) {
               mean_curv_prev = alpha / beta;
               if (mean_curv_prev > max
            } else {
                // Ignore infinite curvature
            }

        }
    }
    */

    /*

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
    */
    /*
    EnergyResults results;
    results.delta_curvature = 0.0;
    results.delta_value = 0.0;
    results.max_curvature = 0.0;
    return results;
    */
}

std::string Morph::frame_fname(std::string frames_dir, int frame) { 
    std::string frame_str = std::to_string(frame);
    return frames_dir + "frame_" + frame_str + ".vdb";
}

void Morph::maybe_save_frame(
        std::string frames_dir, const LevelSet& current_ls, int frame) {
    // Don't do anything if frames_dir is not specified
    if (frames_dir == "")
        return;

    std::string fname = frame_fname(frames_dir, frame);
    std::cout << "Saving frame " << frame << " to " << fname << std::endl;
    current_ls.save(fname);
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
