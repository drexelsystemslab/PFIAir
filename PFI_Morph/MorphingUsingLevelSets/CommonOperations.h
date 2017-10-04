#ifndef COMMONOPERATIONS_H
#define COMMONOPERATIONS_H

#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <sys/stat.h>   //mkdir

namespace CommonOperations { 
    template<class T1, class T2>
    void display(const T1 tag, const T2 msg) {
        tag != "" ? std::cout << tag << " - " << msg << std::endl : std::cout << msg << std::endl;
    }
    
    openvdb::FloatGrid::Ptr getPlatonicVolume(int num_faces, float scale, float voxel_size, float half_width) {
        return openvdb::tools::createLevelSetPlatonic<openvdb::FloatGrid>(num_faces, scale, openvdb::Vec3f(0.0f), voxel_size, half_width);
    }
    
    openvdb::FloatGrid::Ptr getSphereVolume(float radius, float voxel_size, float half_width) {
        return openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(radius, openvdb::Vec3f(0.0f), voxel_size, half_width);
    }
    
    void makeDirs(const char *dir) {
        char tmp[256];
        char *p = NULL;
        size_t len;
        
        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
            tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
            if(*p == '/') {
                *p = 0;
                mkdir(tmp, S_IRWXU);
                *p = '/';
            }
        mkdir(tmp, S_IRWXU);
    }
}

#endif