#include "Morph.h"


MorphStats Morph::morph(
        const LevelSet& source, 
        const LevelSet& target,
        std::string frames_dir,
        int max_iters) {

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
