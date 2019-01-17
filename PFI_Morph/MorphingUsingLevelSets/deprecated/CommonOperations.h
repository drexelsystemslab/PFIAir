#ifndef COMMONOPERATIONS_H
#define COMMONOPERATIONS_H

#include <openvdb/openvdb.h>

#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <sys/stat.h>   //mkdir

/**
 * This namespace holds a number of utility functions.
 */
namespace CommonOperations {
    
    /**
     * Log a message to stdout
     * NOTE: This is only used in Tests.h
     * The format is
     * <tag> - <msg>
     * or
     * <msg> if tag is ""
     */
    template<class T1, class T2>
    void display(const T1 tag, const T2 msg) {
        if (tag != "")
            std::cout << tag << " - " << msg << std::endl;
        else
            std::cout << msg << std::endl;
    }

    
    /**
     * Create a level set of a platonic solid, centered at the origin.
     * This is a shortcut for the OpenVDB version without the center 
     * parameter.
     * 
     * num_faces - number of sides on the platonic solid
     * scale - how much to scale up the shape in world units
     * voxel_size - size of one voxel in world units
     * half_width - half the width of the narrow band around the
     *  surface
     */
    openvdb::FloatGrid::Ptr getPlatonicVolume(
        int num_faces, float scale, float voxel_size, float half_width);

    /**
     * Create a level set of a sphere, centered at the origin.
     * This is a shortcut for the OpenVDB version without the center parameter
     *
     * radius - radius of the sphere
     * voxel_size - size of one voxel in world units
     * half_width - half the width of the narrow band around the
     *  surface
     */
    openvdb::FloatGrid::Ptr getSphereVolume(
        float radius, float voxel_size, float half_width);
    
    /**
     * Make a directory, including any intermediate directories in the path.
     * e.g. makeDirs("foo/bar/baz/") creates
     * foo/
     * foo/bar/
     * foo/bar/baz/
     * 
     * This is much like the Linux command `mkdir -p`
     */
    void makeDirs(const char *dir); 
    
    /**
     * Split a string and return the parts as a vector. This calls
     * split(), defined below.
     */
    std::vector<std::string> prep_do_split(const std::string &s, char delim);

    /**
     * Split a string on a delimiter
     * and store the results in an output collection
     * 
     * Note: This is the low-level version. prep_do_split() is the
     * high-level version
     *
     * Note: result must be an output operator supporting
     * ++ and *. std::back_inserter() is one such iterator used
     * elsewhere in this code.
     */
    template<typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    /**
     * Strip off the extension of a filename.
     */
    std::string getFileNameWithoutExtension(
        std::string file_name, std::string ext);
    
    /**
     * Calculate Euclidean (L2) distance between pt1 and pt2
     */
    double calc3dDistance(openvdb::Vec3d pt1, openvdb::Vec3d pt2);
    
    /**
     * Format a number with commas for the thousands separator
     * the input is an int already cast to a string.
     *
     * TODO: Why not pass in an int and do the casting in the method?
     */
    std::string intNumberFormatCommas(std::string format_str);
}

#endif
