//
//  Orienter.cpp
//  vdbTest
//
//  Created by Liu,Hanjie on 8/30/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//

#include "Orienter.hpp"

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetMorph.h>
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/tools/ValueTransformer.h>

#include <openvdb/math/Maps.h>
#include <openvdb/math/Mat.h>
#include <openvdb/math/Operators.h>
#include <openvdb/math/FiniteDifference.h>

#include <openvdb/util/NullInterrupter.h>

#include <vector>
#include <map>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>

//void calcCentroid(const openvdb::FloatGrid::Ptr& grid_pointer) {
//    
//    openvdb::Vec3d sumWorldCoord(0), centroid(0), currCoord;
//    std::vector<openvdb::Vec3d> inner_coords;
//    int count = 0;
//    for (openvdb::FloatGrid::ValueAllIter iter = grid_pointer->beginValueAll(); iter; ++iter) {
//        if(iter.getValue() < 0) {
//            ++count;
//            currCoord = grid_pointer->indexToWorld(iter.getCoord());
//            inner_coords.push_back(currCoord);
//            sumWorldCoord += currCoord;
//        }
//    }
//    centroid = sumWorldCoord / count;
//    orientGrid(grid_pointer, inner_coords, centroid);
//}

void orientGrid(const openvdb::FloatGrid::Ptr& grid_pointer, std::vector<openvdb::Vec3d> coord_list, openvdb::Vec3d centroid) {
    Eigen::MatrixXd A(3, coord_list.size());
    int col = 0;
    
    for(std::vector<openvdb::Vec3d>::iterator iter = coord_list.begin(); iter != coord_list.end(); iter++) {
        A(0, col) = iter->x() - centroid[0];
        A(1, col) = iter->y() - centroid[1];
        A(2, col) = iter->z() - centroid[2];
        col++;
    }
    
    Eigen::Matrix3d covariance_mat = A * A.transpose();
    Eigen::EigenSolver<Eigen::Matrix3d> es(covariance_mat);
    
    //Eigen::Matrix2d eigen_vals = es.eigenvalues();
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
    
    std::vector<double> sorted_vals = { eigen_vals(0), eigen_vals(1), eigen_vals(2) };
    
    int real_sorted_index[3] = {max_index, mid_index, min_index};
    
    //    int index1 = 0, index2 = 0;
    //
    //
    //    if(real_vals[0] == real_vals[1] && real_vals[0] == real_vals[2]) {
    //        //all eigen vecs are equal
    //        real_sorted_index[0] = 0; real_sorted_index[1] = 1;real_sorted_index[0] = 1;
    //    }
    //    else {
    //        std::sort(sorted_vals.rbegin(), sorted_vals.rend());
    //
    //        for(std::vector<double>::iterator it1 = sorted_vals.begin(); it1 != sorted_vals.end(); ++it1) {
    //            for(std::vector<double>::iterator it2 = real_vals.begin(); it2 != real_vals.end(); ++it2) {
    //                if(*it1 == *it2) {
    //                    if(real_sorted_index[index1] < 0)
    //                        real_sorted_index[index1] = index2;
    //                    //if two eigen vals are equal
    //                    else real_sorted_index[index1] = index2 + 1;
    //                    break;
    //                }
    //                index2++;
    //            }
    //            index1++;
    //            index2 = 0;
    //        }
    //    }
    
    std::cout << eigen_vals << std::endl;
    std::cout << eigen_vecs << std::endl;
    
    Eigen::Matrix<double, 3, 1> z_axis = eigen_vecs.col(real_sorted_index[0]);
    Eigen::Matrix<double, 3, 1> y_axis = eigen_vecs.col(real_sorted_index[1]);
    Eigen::Matrix<double, 3, 1> x_axis = eigen_vecs.col(real_sorted_index[2]);
    
    std::cout << z_axis << std::endl << std::endl;
    std::cout << y_axis << std::endl << std::endl;
    std::cout << x_axis << std::endl << std::endl;
    
    //    Eigen::Matrix3d rot_mat;
    //    rot_mat(0, 0) = z_axis(0); rot_mat(1, 0) = z_axis(1); rot_mat(2, 0) = z_axis(2);
    //    rot_mat(0, 1) = y_axis(0); rot_mat(1, 1) = y_axis(1); rot_mat(2, 1) = y_axis(2);
    //    rot_mat(0, 2) = x_axis(0); rot_mat(1, 2) = x_axis(1); rot_mat(2, 2) = x_axis(2);
    //
    //    Eigen::MatrixXd curr_coord_mat(1, 3);
    //    Eigen::MatrixXd updt_coord_mat(1, 3);
    //    openvdb::Vec3d curr_coord;
    
    openvdb::math::Mat3d rot_mat;
    rot_mat[0][0] = z_axis(0); rot_mat[1][0] = z_axis(1); rot_mat[2][0] = z_axis(2);
    rot_mat[0][1] = y_axis(0); rot_mat[1][1] = y_axis(1); rot_mat[2][1] = y_axis(2);
    rot_mat[0][2] = x_axis(0); rot_mat[1][2] = x_axis(1); rot_mat[2][2] = x_axis(2);
    
    //    openvdb::tools::foreach(grid_pointer->beginValueOn(), MatMul(rot_mat));
    //    writeToFile("rot.vdb", grid_pointer);
    
    //    openvdb::Vec3SGrid::Ptr grid = openvdb::gridPtrCast<openvdb::Vec3SGrid>(readFile("openvdb_cube.vdb"));
    ////    // Construct the rotation matrix.
    //    openvdb::math::Mat3s rot45 =
    //    openvdb::math::rotation<openvdb::math::Mat3s>(openvdb::math::Y_AXIS, M_PI_4);
    //    openvdb::tools::foreach(grid->beginValueOn(), MatMul(rot45));
    //
    //    std::cout << "here";
    
    //grid->setGridClass(openvdb::GRID_LEVEL_SET);
    //    openvdb::io::File file("rot.vdb");
    //    openvdb::GridPtrVec grids;
    //    grids.push_back(grid);
    //    file.write(grid);
    //    file.close();
    
}
