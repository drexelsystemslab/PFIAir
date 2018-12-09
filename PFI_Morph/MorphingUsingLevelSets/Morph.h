#ifndef MORPH_OBJECT_H
#define MORPH_OBJECT_H
#include <memory>
#include <openvdb/util/NullInterrupter.h>
#include <openvdb/tools/LevelSetMorph.h>
#include "LevelSet.h"
#include "MorphStats.h"

/**
 * Morph::calculate_energy() comuptes a few different values,
 * so pack them up in a struct.
 */
struct EnergyResults {
    double delta_curvature;
    double delta_value;
    double max_curvature;
};

/**
 * Morph one model into another and keep track
 * of statistics at each step.
 */
class Morph {
    bool save_frames;
    MorphStats stats;

    // Abbreviations
    typedef openvdb::FloatGrid GridType;
    typedef openvdb::util::NullInterrupter InterruptType;
    typedef openvdb::tools::LevelSetMorphing<GridType, InterruptType> LSMorph;

    // OpenVDB level set morph object
    // Use a pointer because OpenVDB does not provide a null constructor
    std::unique_ptr<LSMorph> ls_morph = nullptr;
public:
    static constexpr int MAX_ITERS = 3;//500;
    static constexpr int NORM_COUNT = 5;
    static constexpr int TIME_STEP = 0.25;

    // Minimum energy consumed per frame. This sets a threshold below
    // which we stop the morphing
    static constexpr int MIN_ENERGY = 10;

    /**
     * Pass in two level sets
     * and run the morping.
     *
     * if frames_dir is specified, frames will be saved
     * to {frames_dir}/advect_XXXX.vdb
     */
    MorphStats morph(
        const LevelSet& source, 
        const LevelSet& target, 
        std::string frames_dir);

    /**
     * Initialize the level set morphing filter
     */
    void init_morph(GridType::Ptr source_grid, GridType::ConstPtr target_grid);

    /**
     * Check if the morph is finished by comparing voxels of two level sets.
     * take the total difference and threshold it.
     */
    bool morph_is_finished(const LevelSet& current, const LevelSet& target);

    /**
     * Compute the inter-frame energy consumption and curvature changes
     */
    EnergyResults calculate_energy(const LevelSet& prev, const LevelSet& next);

    /**
     * format the filename as <frames_dir>/frame_<frame>.vdb
     */
    std::string frame_fname(std::string frames_dir, int frame);
};
#endif