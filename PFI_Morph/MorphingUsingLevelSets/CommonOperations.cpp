#include "CommonOperations.h"

namespace CommonOperations {

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

    std::vector<std::string> prep_do_split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }
    
    std::string getFileNameWithoutExtension(std::string file_name, std::string ext) {
        if(ext == "") ext = ".";
        std::size_t index = file_name.find(ext);
        file_name = file_name.substr(0, index);
        return file_name;
    }

    double calc3dDistance(openvdb::Vec3d pt1, openvdb::Vec3d pt2) {
        
        double sq_sum = openvdb::math::Pow(pt1.x() - pt2.x(), 2) +
                        openvdb::math::Pow(pt1.y() - pt2.y(), 2) +
                        openvdb::math::Pow(pt1.z() - pt2.z(), 2);
        
        return openvdb::math::Sqrt(sq_sum);
    }

    std::string intNumberFormatCommas(std::string format_str) {
        std::vector<std::string> str_arr = CommonOperations::prep_do_split(format_str, '.');
        std::string str = str_arr[0];
        
        std::string decimal = "";
        
        int str_length = (int)str.length() - 1,
        arr_length = str_length + (int)(str_length / 3);
        
        if(str_length < 3) {
            if(str_arr.size() != 2) return str;
            else {
                decimal = str_arr[1];
                if(str.length() == 1) {
                    str += "." + decimal.substr(0, 2);
                }
                else if(str.length() == 2) {
                    str += "." + decimal.substr(0, 1);
                }
                return str;
            }
        }
        
        char char_arr[arr_length];
        
        for(int i = 0, j = 0; i <= str_length; i++, j++) {
            char_arr[j] = str[i];
            if((str_length - i) % 3 == 0 && i != str_length) {
                char_arr[++j] = ',';
            }
        }
        
        std::string return_str(char_arr);
        return return_str.substr(0, arr_length + 1);
    }
}
