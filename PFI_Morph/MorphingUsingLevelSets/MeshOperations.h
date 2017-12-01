#ifndef MESHOPERATIONS_H
#define MESHOPERATIONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <dirent.h>
#include <algorithm>

#include "CommonOperations.h"
#include "Container.hpp"
#include "Algorithm.hpp"

namespace MeshOperations {
    void calcBoundingBox(std::vector<Eigen::Matrix<double, 1, 4>>, std::vector<double>&, openvdb::Vec3d&);
    void adjustScale(std::vector<Eigen::Matrix<double, 1, 4>>&, double);
    
    void readOBJ(std::string filepath, std::vector<std::vector<std::string>>& v_list, std::vector<std::vector<std::string>>& vn_list, std::vector<std::vector<std::string>>& f_list) {
        
        std::ifstream infile(filepath);
        std::string line;
        
        std::vector<std::string> items;
        
        
        while (std::getline(infile, line))
        {
            items = CommonOperations::prep_do_split(line, ' ');
            
            if(items.size()) {
                if(items[0] == "v") {
                    v_list.push_back(items);
                }
                else if(items[0] == "vn") {
                    vn_list.push_back(items);
                }
                else if(items[0] == "f")
                    f_list.push_back(items);
            }
        }
    }
    
    void writeOBJ(std::string filename, std::vector<Eigen::Matrix<double, 1, 4>> vertices, std::vector<std::vector<std::string>>& f_list, bool includeAxis) {
        std::ofstream file;
        file.open (filename);
        
        for(int i = 0; i < vertices.size(); i++) {
            file << "v " << vertices[i](0, 0) << " " << vertices[i](0, 1) << " " << vertices[i](0, 2) << "\n";
        }
        
        for(int i = 0; i < f_list.size(); i++) {
            file << "f " << f_list[i][2] << " " << f_list[i][3] << " " << f_list[i][4] << "\n";
        }
        
        if(includeAxis) {
            
            file << "f " << vertices.size() - 6 << "//" << vertices.size() - 6 << " "
            << vertices.size() - 5 << "//" << vertices.size() - 5 << " "
            << vertices.size() - 4 << "//" << vertices.size() - 4 << "\n";
            
            file << "f " << vertices.size() - 6 << "//" << vertices.size() - 6 << " "
            << vertices.size() - 3 << "//" << vertices.size() - 3 << " "
            << vertices.size() - 2 << "//" << vertices.size() - 2 << "\n";
            
            file << "f " << vertices.size() - 6 << "//" << vertices.size() - 6 << " "
            << vertices.size() - 1 << "//" << vertices.size() - 1 << " "
            << vertices.size() << "//" << vertices.size() << "\n";
        }
        
        file.close();
    }
    
    void calcCentroid(std::vector<Eigen::Matrix<double, 1, 4>> vertices, Eigen::Matrix<double, 1, 4>& centroid) {
        if(!vertices.size()) return;
        
        Eigen::Matrix<double, 1, 4> sum_coords = vertices[0];
        
        for(int i = 1; i < vertices.size(); i++) {
            sum_coords += vertices[i];
        }
        
        sum_coords /= vertices.size();
        centroid = sum_coords;
    }
    
    void performPCA(std::vector<std::vector<std::string>> v_list, std::vector<Eigen::Matrix<double, 1, 4>>& v_mat_list, Eigen::Matrix4d& rot_mat) {
        //MatrixXd requires size before hand
        Eigen::MatrixXd A(3, v_list.size());
        Eigen::Matrix<double, 1, 4> list_item;
        
        int col = 0;
        
        for(int i = 0; i < v_list.size(); i++) {
            
            list_item(0, 0) = std::stod(v_list[i][2]);
            list_item(0, 1) = std::stod(v_list[i][3]);
            list_item(0, 2) = std::stod(v_list[i][4]);
            list_item(0, 3) = 1.0;
            
            v_mat_list.push_back(list_item);
            
            A(0, col) = std::stod(v_list[i][2]);
            A(1, col) = std::stod(v_list[i][3]);
            A(2, col) = std::stod(v_list[i][4]);
            col++;
        }
        
        std::vector<double> axis_lengths;
        openvdb::Vec3d center(0, 0, 0);
        calcBoundingBox(v_mat_list, axis_lengths, center);
        double max_length = openvdb::math::Max(axis_lengths[0], axis_lengths[1]);
        max_length = openvdb::math::Max(max_length, axis_lengths[2]);
        
        if(max_length < 1.0) adjustScale(v_mat_list, max_length);
        
        Eigen::Matrix3d covariance_mat = A * A.transpose();
        Eigen::EigenSolver<Eigen::Matrix3d> es(covariance_mat);
        
        Eigen::Matrix<double, 3, 1> eigen_vals = es.eigenvalues().col(0).real();
        Eigen::Matrix<double, 3, 3> eigen_vecs = es.eigenvectors().real();
        
        std::vector<double> real_vals = { eigen_vals(0), eigen_vals(1), eigen_vals(2) };
        
        int max = real_vals[0], max_index = 0, mid_index = 0, min_index = 0;
        
        for(int i = 1 ; i < real_vals.size(); i++) {
            if(real_vals[i] > max) {
                max = real_vals[i];
                max_index = i;
            }
        }
        
        switch(max_index) {
            case 0:
                if(real_vals[1] < real_vals[2]) { min_index = 1; mid_index = 2; }
                else { min_index = 2; mid_index = 1; }
                break;
                
            case 1:
                if(real_vals[0] < real_vals[2]) { min_index = 0; mid_index = 2; }
                else { min_index = 2; mid_index = 0; }
                break;
                
            case 2:
                if(real_vals[0] < real_vals[1]) { min_index = 0; mid_index = 1; }
                else { min_index = 1; mid_index = 0; }
                break;
        }
        
        int real_sorted_index[3] = {max_index, mid_index, min_index};
        
        Eigen::Matrix<double, 3, 1> z_axis_vec = eigen_vecs.col(real_sorted_index[0]);
        Eigen::Matrix<double, 3, 1> y_axis_vec = eigen_vecs.col(real_sorted_index[1]);
        Eigen::Matrix<double, 3, 1> x_axis_vec = eigen_vecs.col(real_sorted_index[2]);
        
        Eigen::Matrix<double, 3, 1> cross_vec = {
            (z_axis_vec(1) * y_axis_vec(2)) - (z_axis_vec(2) * y_axis_vec(1)),
            (z_axis_vec(2) * y_axis_vec(0)) - (z_axis_vec(0) * y_axis_vec(2)),
            (z_axis_vec(0) * y_axis_vec(1)) - (z_axis_vec(1) * y_axis_vec(0)),
        };

//        Eigen::Matrix<double, 3, 1> cross_vec = x_axis_vec;
        
        rot_mat <<  cross_vec(0), y_axis_vec(0), z_axis_vec(0), 0,
                    cross_vec(1), y_axis_vec(1), z_axis_vec(1), 0,
                    cross_vec(2), y_axis_vec(2), z_axis_vec(2), 0,
                    0, 0, 0, 1;
        
    }
    
    
    void calcBoundingBox(std::vector<Eigen::Matrix<double, 1, 4>> vertices, std::vector<double>& axis_lengths, openvdb::Vec3d& center) {
        double limit = openvdb::math::Pow(10, 10);
        double min_x = limit, max_x = -limit, min_y = limit, max_y = -limit, min_z = limit, max_z = -limit, x, y, z;
        
        for(int i = 0; i < vertices.size(); i++) {
            x = vertices[i](0, 0);
            y = vertices[i](0, 1);
            z = vertices[i](0, 2);
            
            if(x < min_x) min_x = x;
            else if(x > max_x) max_x = x;
            
            if(y < min_y) min_y = y;
            else if(y > max_y) max_y = y;
            
            if(z < min_z) min_z = z;
            else if(z > max_z) max_z = z;
            
        }
        axis_lengths.push_back(fabs(max_x - min_x));
        axis_lengths.push_back(fabs(max_y - min_y));
        axis_lengths.push_back(fabs(max_z - min_z));
        
        center[0] = (min_x + max_x) / 2;
        center[1] = (min_y + max_y) / 2;
        center[2] = (min_z + max_z) / 2;
    }
    
    void adjustScale(std::vector<Eigen::Matrix<double, 1, 4>>& vertices1, double z1) {
        for(int i = 0; i < vertices1.size(); i++) {
            vertices1[i](0, 0) /= z1;
            vertices1[i](0, 1) /= z1;
            vertices1[i](0, 2) /= z1;
        }
    }
    
    void translateCenterToOrigin(std::vector<Eigen::Matrix<double, 1, 4>>& vertices, openvdb::Vec3d center) {
        
        Eigen::Matrix<double, 4, 1> temp;
        Eigen::Matrix4d trans;
        trans <<
        1, 0, 0, -center.x(),
        0, 1, 0, -center.y(),
        0, 0, 1, -center.z(),
        0, 0, 0, 1;
        
        for(int i = 0; i < vertices.size(); i++) {
            temp = trans * vertices[i].transpose();
            vertices[i] = temp.transpose();
            vertices[i](3) = 1.0;
        }
    }
    
    void convertMeshToVolume(std::string obj_filename, std::string vdb_filename, std::string write_path, float bandwidth, double voxel_size) {
        PFIAir::Container model = PFIAir::Container();
        
        model.loadMeshModel(obj_filename);
        model.computeMeshCenter();
        
        model.setScale(openvdb::Vec3d(voxel_size));
        
        auto shape = model.getWaterTightLevelSetWithBandWidth(bandwidth);
        model.exportModel(write_path + vdb_filename, shape);
    }
    
    void addPCAAxisToVertices(std::vector<Eigen::Matrix<double, 1, 4>>& vertices1, openvdb::Vec3d center1, Eigen::Matrix4d rot_mat1, double max_length) {
        
        Eigen::Matrix<double, 1, 4> list_item;
        
        list_item << 0.0, 0.0, 0.0, 1.0;
        vertices1.push_back(list_item);
        
        //double alpha = 1.2 * max_length;
        
        for(int i = 0; i < 3; i++) {
            list_item <<
            center1.x() + (max_length * (3 - i) * rot_mat1(0, 2 - i)),
            center1.y() + (max_length * (3 - i) * rot_mat1(1, 2 - i)),
            center1.z() + (max_length * (3 - i) * rot_mat1(2, 2 - i)),
            1.0;
            
            vertices1.push_back(list_item);
            
            list_item <<
            center1.x() + (max_length * (3 - i) * rot_mat1(0, 2 - i)),
            center1.y() + (max_length * (3 - i) * rot_mat1(1, 2 - i) + 0.1),
            center1.z() + (max_length * (3 - i) * rot_mat1(2, 2 - i)),
            1.0;
            
            vertices1.push_back(list_item);
        }
    }
    
    
    double calcSkewness(const std::vector<Eigen::Matrix<double, 1, 4>> vertices, bool pcaIncluded) {
        if(vertices.size() == 0) return 0;
        
        int vertices_size = pcaIncluded ? vertices.size() - 6 : vertices.size();
        double z_values[vertices_size],
        z_total = 0.0,
        current_count = 0.0,
        max_count = 1.0,
        assoc_key = vertices[0](2);
        
        std::map<double, int> map_z_count;
        
        
        for(int i = 0; i < vertices_size; i++) {
            z_values[i] = vertices[i](2);
            z_total += vertices[i](2);
            
            if(map_z_count.find(vertices[i](2)) != map_z_count.end()) {
                current_count = map_z_count[vertices[i](2)];
                current_count++;
                if(max_count < current_count) {
                    max_count = current_count;
                    assoc_key = vertices[i](2);
                }
                map_z_count[vertices[i](2)] = current_count;
            }
            else {
                map_z_count[vertices[i](2)] = 1;
            }
        }
        double mean = z_total / vertices_size;
        //skewness = mean - assoc_key;    //assoc_key -> mode
        
        std::sort(z_values, z_values + vertices_size);
        
        double median;
        if(vertices_size % 2 == 0) {
            median = (z_values[vertices_size/2] + z_values[vertices_size/2 - 1]) / 2;
        }
        else median = z_values[vertices_size/2];
        
        double skewness = mean - median;
        std::cout << max_count << std::endl;
        std::cout << assoc_key << std::endl;
        std::cout << skewness << std::endl;
        
        return skewness;
    }
    
    void rotateAcrossYAxis(std::vector<Eigen::Matrix<double, 1, 4>>& vertices) {
        Eigen::Matrix4d rot_mat;
        rot_mat <<
        -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1;
        
        for(int i = 0; i < vertices.size(); i++)
            vertices[i] *= rot_mat;
    }
    
    void performPCAAndDrawAxis(std::string filepath1, std::string write_name) {
        std::vector<std::vector<std::string>> v_list;
        std::vector<std::vector<std::string>> vn_list;
        std::vector<std::vector<std::string>> f_list1;
        std::vector<Eigen::Matrix<double, 1, 4>> vertices1;
        Eigen::Matrix4d rot_mat1;
        std::vector<double> axis_lengths1;
        openvdb::Vec3d center1(0, 0, 0);
        Eigen::Matrix<double, 1, 4> list_item;
        
        MeshOperations::readOBJ(filepath1, v_list, vn_list, f_list1);
        MeshOperations::performPCA(v_list, vertices1, rot_mat1);
        
        for(int i = 0; i < vertices1.size(); i++)
            vertices1[i] *= rot_mat1;
        
        MeshOperations::calcBoundingBox(vertices1, axis_lengths1, center1);
        double max_length1 = openvdb::math::Max(axis_lengths1[0], axis_lengths1[1]);
        max_length1 = openvdb::math::Max(max_length1, axis_lengths1[2]);
        addPCAAxisToVertices(vertices1, center1, rot_mat1, max_length1);
        
        for(int i = vertices1.size() - 6; i < vertices1.size(); i++)
            vertices1[i] *= rot_mat1;
        
        if(calcSkewness(vertices1, true) > 0)
            rotateAcrossYAxis(vertices1);
        
        MeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
        
    }
    
    
    void doAllMeshOperations(std::string filepath1, std::string filepath2) {
        
        std::vector<std::vector<std::string>> v_list;
        std::vector<std::vector<std::string>> vn_list;
        std::vector<std::vector<std::string>> f_list1, f_list2;
        std::vector<Eigen::Matrix<double, 1, 4>> vertices1, vertices2;
        Eigen::Matrix4d rot_mat1, rot_mat2;
        std::vector<double> axis_lengths1, axis_lengths2;
        openvdb::Vec3d center1(0, 0, 0), center2(0, 0, 0);
        Eigen::Matrix<double, 1, 4> list_item;
        bool includePCA = false;
        
        MeshOperations::readOBJ(filepath1, v_list, vn_list, f_list1);
        MeshOperations::performPCA(v_list, vertices1, rot_mat1);
        
        for(int i = 0; i < vertices1.size(); i++)
            vertices1[i] *= rot_mat1;
        
        MeshOperations::calcBoundingBox(vertices1, axis_lengths1, center1);
        
        if(includePCA) {
            double max_length1 = openvdb::math::Max(axis_lengths1[0], axis_lengths1[1]);
            max_length1 = openvdb::math::Max(max_length1, axis_lengths1[2]);
            addPCAAxisToVertices(vertices1, center1, rot_mat1, max_length1);
            
            for(int i = vertices1.size() - 6; i < vertices1.size(); i++)
                vertices1[i] *= rot_mat1;
        }
        
        if(calcSkewness(vertices1, includePCA) > 0)
            rotateAcrossYAxis(vertices1);
        
        v_list.clear();
        vn_list.clear();
        
        MeshOperations::readOBJ(filepath2, v_list, vn_list, f_list2);
        MeshOperations::performPCA(v_list, vertices2, rot_mat2);
        
        for(int i = 0; i < vertices2.size(); i++)
            vertices2[i] *= rot_mat2;
        
        MeshOperations::calcBoundingBox(vertices2, axis_lengths2, center2);
        
        if(includePCA) {
            double max_length2 = openvdb::math::Max(axis_lengths2[0], axis_lengths2[1]);
            max_length2 = openvdb::math::Max(max_length2, axis_lengths2[2]);
            addPCAAxisToVertices(vertices2, center2, rot_mat2, max_length2);
            
            for(int i = vertices2.size() - 6; i < vertices2.size(); i++)
                vertices2[i] *= rot_mat2;

        }
        
        if(calcSkewness(vertices2, includePCA) > 0)
            rotateAcrossYAxis(vertices2);
        
        MeshOperations::translateCenterToOrigin(vertices1, center1);
        MeshOperations::translateCenterToOrigin(vertices2, center2);
        
        MeshOperations::adjustScale(vertices1, axis_lengths1[2]);
        MeshOperations::adjustScale(vertices2, axis_lengths2[2]);
        
        MeshOperations::writeOBJ("srt1.obj", vertices1, f_list1, includePCA);
        MeshOperations::writeOBJ("srt2.obj", vertices2, f_list2, includePCA);

    }
    
    void performActionForAllObjs() {
        DIR *dir1;
        struct dirent *ent1;
        std::string obj_path1 = "original_objs/";
        
        if ((dir1 = opendir ("original_objs")) != NULL) {
            while ((ent1 = readdir (dir1)) != NULL) {
                if(ent1->d_type == DT_REG) {
                    MeshOperations::performPCAAndDrawAxis(obj_path1 + ent1->d_name, ent1->d_name);
                }
            }
        }
    }
}

#endif

