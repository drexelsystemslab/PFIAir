#include "Morph.h"
#include <cmath>

MorphStats Morph::morph(
        const LevelSet& source, 
        const LevelSet& target,
        std::string frames_dir,
        int max_iters) {

    // Compute the maximum finite curvature for each model
    double source_max_curv = EnergyCalculator::compute_max_curvature(source);
    double target_max_curv = EnergyCalculator::compute_max_curvature(target);
    max_curvature = std::max(source_max_curv, target_max_curv);

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
    for (frame_count = 1; frame_count < max_iters; frame_count++) {
        // Check if we've consumed too much energy
        if (energy_consumed < MIN_ENERGY)
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
        stats.update_energy(energy.delta_curvature, energy.delta_value);
        stats.update_max_curvature(energy.max_curvature);  

        // Output summary of frame
        std::cout << "\nFrame " << frame_count << " -----------" << std::endl;
        std::cout << "CFL iterations - " << cfl_iters << std::endl;
        std::cout << "(dCurvature, dValue, max_Curvature) = ("
            << energy.delta_curvature << ", " << energy.delta_value << ", "
            << energy.max_curvature << ")" << std::endl;

        // Optionally save the frame
        maybe_save_frame(frames_dir, current_ls, frame_count);

        // Check if the morph is finished
        if (morph_is_finished(prev_ls, current_ls, target))
            break;
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
        const LevelSet& prev, 
        const LevelSet& current, 
        const LevelSet& target) {

    // Get the underlying grids
    openvdb::FloatGrid::ConstPtr prev_grid = prev.get_level_set();
    openvdb::FloatGrid::ConstPtr current_grid = current.get_level_set();
    openvdb::FloatGrid::ConstPtr target_grid = target.get_level_set();

    // We care about values that have moved by at least a voxel
    double threshold = target.get_voxel_size();

    // We want to compute the maximum difference of voxels along
    // with the number of voxels that changed between each pair
    // of frames (prev, curr), (curr, target)
    double max_diff_target = 0.0;
    double max_diff_prev = 0.0;
    //int voxels_diff_from_target = 0;
    //int voxels_diff_from_prev = 0;

    // We need the accessors for the grids we are not iterating over
    openvdb::FloatGrid::ConstAccessor prev_acc = prev_grid->getAccessor();
    openvdb::FloatGrid::ConstAccessor target_acc = target_grid->getAccessor();

    // Iterate over the current grid and compare to the others
    SurfaceIterator surf_iter = current.get_surface_iterator();
    for (surf_iter.begin(); surf_iter; surf_iter++) {
        // Lookup the value in all three grids
        openvdb::Coord coord = surf_iter.get_coord();
        double prev_val = prev_acc.getValue(coord);
        double curr_val = surf_iter.get_value(); 
        double target_val = target_acc.getValue(coord);

        // Compute the absolute difference in value, i.e. the distance
        // the voxel moved
        double diff_prev = std::abs(curr_val - prev_val);
        double diff_target = std::abs(curr_val - target_val);

        // If the value changed by a distance of at least 1 voxel
        // increment the appropriate counter
        /*
        if (diff_prev > threshold)
            voxels_diff_from_prev++;
        if (diff_target > threshold)
            voxels_diff_from_target++;
        */

        // Update the maximum difference values
        if (diff_prev > max_diff_prev)
            max_diff_prev = diff_prev;
        if (diff_target > max_diff_target) 
            max_diff_target = diff_target;
    }

    // Threshold the result
    /*
    std::cout << "Max diff prev -> curr - " << max_diff_prev << std::endl;
    std::cout << "Changed Voxels prev -> curr - " 
        << voxels_diff_from_prev << std::endl;
    std::cout << "Max diff curr -> target - " << max_diff_target << std::endl;
    std::cout << "Changed Voxels curr -> target - " 
        << voxels_diff_from_target << std::endl;
    std::cout << "Threshold - " << threshold << std::endl;
    */

    // Update Morph Stats
    //stats.add_voxels_diff(voxels_diff_from_prev, voxels_diff_from_target);
    stats.add_max_diffs(max_diff_prev, max_diff_target);

    // TODO: Exact calculation TBD by results of above
    return max_diff_target < threshold || max_diff_prev < 0.5 * threshold;
}

EnergyResults Morph::calculate_energy(
        const LevelSet& prev, const LevelSet& curr) {

    EnergyCalculator calc(prev, curr);
    return calc.compute_energy(max_curvature);
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
