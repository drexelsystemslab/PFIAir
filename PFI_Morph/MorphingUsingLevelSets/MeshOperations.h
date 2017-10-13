#ifndef MESHOPERATIONS_H
#define MESHOPERATIONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include "CommonOperations.h"

namespace MeshOperations {
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
        
        Eigen::Matrix<double, 3, 1> z_axis = eigen_vecs.col(real_sorted_index[0]);
        Eigen::Matrix<double, 3, 1> y_axis = eigen_vecs.col(real_sorted_index[1]);
        Eigen::Matrix<double, 3, 1> x_axis = eigen_vecs.col(real_sorted_index[2]);
        
        
        rot_mat <<  x_axis(0), y_axis(0), z_axis(0), 0,
                    x_axis(1), y_axis(1), z_axis(1), 0,
                    x_axis(2), y_axis(2), z_axis(2), 0,
                    0, 0, 0, 1;
        std::cout << rot_mat << std::endl;
//        rot_mat(0, 0) = x_axis(0); rot_mat(1, 0) = x_axis(1); rot_mat(2, 0) = x_axis(2); rot_mat(3, 0) = 0;
//        rot_mat(0, 1) = y_axis(0); rot_mat(1, 1) = y_axis(1); rot_mat(2, 1) = y_axis(2); rot_mat(3, 1) = 0;
//        rot_mat(0, 2) = z_axis(0); rot_mat(1, 2) = z_axis(1); rot_mat(2, 2) = z_axis(2); rot_mat(3, 2) = 0;
//        rot_mat(0, 3) = 0; rot_mat(1, 3) = 0; rot_mat(2, 3) = 0; rot_mat(3, 3) = 1;
    }
    
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
    
    void writeOBJ(std::string filename, std::vector<Eigen::Matrix<double, 1, 4>> vertices, std::vector<std::vector<std::string>>& f_list) {
        std::ofstream file;
        file.open (filename);
        
        for(int i = 0; i < vertices.size(); i++) {
            file << "v " << vertices[i](0, 0) << " " << vertices[i](0, 1) << " " << vertices[i](0, 2) << "\n";
        }
        
        for(int i = 0; i < f_list.size(); i++) {
            file << "f " << f_list[i][2] << " " << f_list[i][3] << " " << f_list[i][4] << "\n";
        }
        
        file.close();
    }
    
    
    void calcBoundingBox(std::vector<Eigen::Matrix<double, 1, 4>> vertices, std::vector<double>& axis_lengths) {
        double min_x = 0, max_x = 0, min_y = 0, max_y = 0, min_z = 0, max_z = 0, x, y, z;
        
        for(int i = 0; i < vertices.size(); i++) {
            x = vertices[i](0, 0);
            y = vertices[i](0, 1);
            z = vertices[i](0, 2);
            
            if(x < min_x) min_x = x;
            else if(x > max_x) max_x = x;
            
            if(y < min_y) min_y = y;
            else if(y > max_x) max_y = y;
            
            if(z < min_z) min_z = z;
            else if(z > max_z) max_z = z;
            
        }
        axis_lengths.push_back(fabs(max_x - min_x));
        axis_lengths.push_back(fabs(max_y - min_y));
        axis_lengths.push_back(fabs(max_z - min_z));
    }
    
    void adjustScale(std::vector<Eigen::Matrix<double, 1, 4>>& vertices1, std::vector<Eigen::Matrix<double, 1, 4>>& vertices2, double z1, double z2) {
        
        for(int i = 0; i < vertices1.size(); i++) {
            vertices1[i](0, 0) /= z1;
            vertices1[i](0, 1) /= z1;
            vertices1[i](0, 2) /= z1;
        }
        
        for(int i = 0; i < vertices2.size(); i++) {
            vertices2[i](0, 0) /= z2;
            vertices2[i](0, 1) /= z2;
            vertices2[i](0, 2) /= z2;
        }
    }
    
    void translateCentroidToOrigin(std::vector<Eigen::Matrix<double, 1, 4>>& vertices1, std::vector<Eigen::Matrix<double, 1, 4>>& vertices2) {
        Eigen::Matrix<double, 1, 4> centroid1, centroid2;
        Eigen::Matrix<double, 4, 1> temp;
        
        calcCentroid(vertices1, centroid1);
        calcCentroid(vertices2, centroid2);
        
        Eigen::Matrix4d trans1, trans2;
        trans1 <<   1, 0, 0, -centroid1(0, 0),
        0, 1, 0, -centroid1(0, 1),
        0, 0, 1, -centroid1(0, 2),
        0, 0, 0, 1;
        
        trans2 <<   1, 0, 0, -centroid2(0, 0),
        0, 1, 0, -centroid2(0, 1),
        0, 0, 1, -centroid2(0, 2),
        0, 0, 0, 1;
        
        for(int i = 0; i < vertices1.size(); i++) {
            temp = trans1 * vertices1[i].transpose();
            vertices1[i] = temp.transpose();
            vertices1[i](3) = 1.0;
        }
        
        for(int i = 0; i < vertices2.size(); i++) {
            temp = trans2 * vertices2[i].transpose();
            vertices2[i] = temp.transpose();
            vertices2[i](3) = 1.0;
        }
    }
}

#endif