#ifndef COMMONOPERATIONS_H
#define COMMONOPERATIONS_H

#include <openvdb/openvdb.h>

#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <sys/stat.h>   //mkdir

namespace CommonOperations {
    
    template<class T1, class T2>
    void display(const T1 tag, const T2 msg) {
        tag != "" ? std::cout << tag << " - " << msg << std::endl : std::cout << msg << std::endl;
    }
    
    openvdb::FloatGrid::Ptr getPlatonicVolume(
        int num_faces, float scale, float voxel_size, float half_width);

    openvdb::FloatGrid::Ptr getSphereVolume(
        float radius, float voxel_size, float half_width);
    
    void makeDirs(const char *dir);
    
    
    template<typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }
    
    std::vector<std::string> prep_do_split(const std::string &s, char delim);

    std::string getFileNameWithoutExtension(
        std::string file_name, std::string ext);
    
    double calc3dDistance(openvdb::Vec3d pt1, openvdb::Vec3d pt2);
    
    std::string intNumberFormatCommas(std::string format_str);
}

#endif
