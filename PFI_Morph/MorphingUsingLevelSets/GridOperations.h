#ifndef GRIDOPERATIONS_H
#define GRIDOPERATIONS_H

#include <openvdb/util/NullInterrupter.h>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <fstream>

#include "CommonOperations.h"

namespace GridOperations {
    
    struct MatMul {
        openvdb::math::Mat3s M;
        MatMul(const openvdb::math::Mat3s& mat): M(mat) {}
        inline void operator()(const openvdb::Vec3SGrid::ValueOnIter& iter) const {
            try {
                iter.setValue(M.transform(*iter));
            }
            catch(...) {
                
            }
        }
    };
    
    openvdb::GridBase::Ptr readFile(const std::string);
    void writeToFile(const std::string, openvdb::FloatGrid::Ptr);
    void writeOnlySurface(const openvdb::FloatGrid::Ptr);
    
    double measureGrid(const openvdb::FloatGrid::Ptr);
    void adjustScale(const openvdb::FloatGrid::Ptr&, const openvdb::FloatGrid::Ptr&);
    double calcPlatonicScale(int, double);
    void calcCentroid(const openvdb::FloatGrid::Ptr&);
    void orientGrid(const openvdb::FloatGrid::Ptr&, const std::vector<openvdb::Vec3d>, openvdb::Vec3d);
    
    bool checkIfSurface(openvdb::FloatGrid::ValueOnIter, const openvdb::FloatGrid::Ptr);
    
    
    openvdb::GridBase::Ptr readFile(const std::string file_name) {
        openvdb::io::File file(file_name);
        file.open();
        openvdb::GridBase::Ptr baseGrid;
        for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
        {
            baseGrid = file.readGrid(nameIter.gridName());
        }
        file.close();
        return baseGrid;
    }
    
    void writeToFile(const std::string file_name, openvdb::FloatGrid::Ptr grid_pointer) {
        grid_pointer->setGridClass(openvdb::GRID_LEVEL_SET);
        openvdb::io::File file(file_name);
        openvdb::GridPtrVec grids;
        grids.push_back(grid_pointer);
        file.write(grids);
        file.close();
    }
    
    double measureGrid(const openvdb::FloatGrid::Ptr grid_pointer) {
        openvdb::util::NullInterrupter* interrupt = nullptr;
        openvdb::tools::LevelSetMeasure <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_measure(*(grid_pointer), interrupt);
        
        double area = 0.0, volume = 0.0;
        ls_measure.measure(area, volume);
        
        return area;
    }
    
    
    void adjustScale(const openvdb::FloatGrid::Ptr& source_pointer, const openvdb::FloatGrid::Ptr& target_pointer) {
        //openvdb::FloatGrid::Ptr scaled_source_grid;
        double  source_area, target_area, area_ratio, source_scale_sum = 0, target_scale_sum = 0;
        int count = 0;
        
        source_area = measureGrid(source_pointer);
        target_area = measureGrid(target_pointer);
        area_ratio =  source_area / target_area;
        
        while(!(area_ratio >= 0.95 && area_ratio <= 1.05)) {           //greater than +/- 5%
            if(++count == 3) {
                if(!(area_ratio >= 0.95 && area_ratio <= 1.05)) std::cout << "scaling not under 5%" << std::endl;
                break;
            }
            
            //sclaing up
            if(source_area < target_area) {
                //use deep copy instead later on
                source_scale_sum += target_area / source_area;
                source_pointer->setTransform(openvdb::math::Transform::createLinearTransform(source_pointer->voxelSize()[0]));
                source_pointer->transform().postScale(source_scale_sum);
                source_pointer->pruneGrid();
                //writeToFile("scaled_souce.vdb", source_pointer);
            }
            else {
                //use deep copy instead later on
                target_scale_sum += source_area / target_area;
                target_pointer->setTransform(openvdb::math::Transform::createLinearTransform(target_pointer->voxelSize()[0]));
                target_pointer->transform().postScale(target_scale_sum);
                target_pointer->pruneGrid();
                //writeToFile("scaled_target.vdb", target_pointer);
            }
            
            source_area = measureGrid(source_pointer);
            target_area = measureGrid(target_pointer);
            area_ratio =  source_area / target_area;
            
        }
        
    }
    
    void calcCentroid(const openvdb::FloatGrid::Ptr& grid_pointer) {
        
        openvdb::Vec3d sumWorldCoord(0), centroid(0), currCoord;
        std::vector<openvdb::Vec3d> inner_coords;
        int count = 0;
        for (openvdb::FloatGrid::ValueAllIter iter = grid_pointer->beginValueAll(); iter; ++iter) {
            if(iter.getValue() < 0) {
                ++count;
                currCoord = grid_pointer->indexToWorld(iter.getCoord());
                inner_coords.push_back(currCoord);
                sumWorldCoord += currCoord;
            }
        }
        centroid = sumWorldCoord / count;
        orientGrid(grid_pointer, inner_coords, centroid);
    }
    
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
    
    bool checkIfSurface(openvdb::FloatGrid::ValueOnIter iterator, const openvdb::FloatGrid::Ptr grid_pointer) {
        bool found_positive = false;
        openvdb::Coord coord = iterator.getCoord();
        openvdb::Coord six_connected[6] = {
            openvdb::Coord(coord.x() - 1, coord.y(), coord.z()),
            openvdb::Coord(coord.x() + 1, coord.y(), coord.z()),
            openvdb::Coord(coord.x(), coord.y() - 1, coord.z()),
            openvdb::Coord(coord.x(), coord.y() + 1, coord.z()),
            openvdb::Coord(coord.x(), coord.y(), coord.z() - 1),
            openvdb::Coord(coord.x(), coord.y(), coord.z() + 1)
        };
        
        openvdb::FloatGrid::Accessor accessor = grid_pointer->getAccessor();
        
        for(int i = 0; i < 6; i++) {
            if(accessor.getValue(six_connected[i]) > 0) {
                found_positive = true;
                break;
            }
        }
        return found_positive;
    } 
    
    void writeOnlySurface(const openvdb::FloatGrid::Ptr grid_pointer) {
        double bg = grid_pointer->background();
        int surface = 0, non_surface = 0;
        std::cout << grid_pointer->activeVoxelCount() << std::endl;
        for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0) {
                if(!GridOperations::checkIfSurface(iter, grid_pointer)) {
                    ++non_surface;
                    iter.setValue(bg);
                }
                else ++surface;
            }
        }
        std::cout << surface << std::endl;
        std::cout << non_surface << std::endl;
        GridOperations::writeToFile("only_surface.vdb", grid_pointer);
    }
    
    double calcPlatonicScale(int face, double radius) {
        double _pi = 3.14159,
        sa_sphere = 4 * _pi * openvdb::math::Pow(radius, 2),
        ret_scale = 1.0;
        
        double openvdb_platonic_def_side_length_arr[5] = {1.632991, 1.0, 1.414214, 0.708876, 1.051463};
        
        switch(face) {
            case 4:
                ret_scale = openvdb::math::Sqrt(sa_sphere / (openvdb::math::Sqrt(3.0) * openvdb::math::Pow(openvdb_platonic_def_side_length_arr[0], 2)));
                break;
            case 6:
                ret_scale =  openvdb::math::Sqrt(sa_sphere / (6 * openvdb::math::Pow(openvdb_platonic_def_side_length_arr[1], 2)));
                break;
            case 8:
                ret_scale = openvdb::math::Sqrt(sa_sphere / (2 * openvdb::math::Sqrt(3.0) * openvdb::math::Pow(openvdb_platonic_def_side_length_arr[2], 2)));
                break;
            case 12:
                ret_scale = openvdb::math::Sqrt(sa_sphere / (20.6457288071 * openvdb::math::Pow(openvdb_platonic_def_side_length_arr[3], 2)));
                break;
            case 20:
                ret_scale = openvdb::math::Sqrt(sa_sphere / (5 * openvdb::math::Sqrt(3.0) * openvdb::math::Pow(openvdb_platonic_def_side_length_arr[4], 2)));
                break;
        }
        return ret_scale;
    }
    
    void writeActiveVoxelVals() {
        double sphere_radius = 5.0, scale = 5.0, voxel_size = 0.03, half_width = 30.0;
        openvdb::FloatGrid::Ptr grids[6] = {CommonOperations::getSphereVolume(sphere_radius, voxel_size, half_width),
            CommonOperations::getPlatonicVolume(4, scale, voxel_size, half_width),
            CommonOperations::getPlatonicVolume(6, scale, voxel_size, half_width),
            CommonOperations::getPlatonicVolume(8, scale, voxel_size, half_width),
            CommonOperations::getPlatonicVolume(12, scale, voxel_size, half_width),
            CommonOperations::getPlatonicVolume(20, scale, voxel_size, half_width)};
        
        std::string labels[6] = {"Sphere", "Tetra", "Cube", "Octa", "Dodec", "Icoso"};
        std::string filename = "";
        std::ofstream file;
        openvdb::Coord curr_coord;
        
        file.open ("active_vals.txt");
        file << "Sphere radius - " << sphere_radius << "\n";
        file << "Scale - " << scale << "\n";
        file << "Voxel size - " << voxel_size << "\n";
        file << "Half width - " << half_width << "\n\n";
        
        for(int i=0; i < 6; i++) {
            openvdb::FloatGrid::Ptr grid_pointer = grids[i];
            filename = "output/stock_vdbs/r" + std::to_string((int)sphere_radius) + "s" + std::to_string((int)scale) + "/" + labels[i] + ".vdb";
            GridOperations::writeToFile(filename, grid_pointer);
            
            file << "For level set " << labels[i] << "\n";
            for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
                curr_coord = iter.getCoord();
                if(curr_coord.z() == 0)
                    file << "At Coord: " << curr_coord << " = " << iter.getValue() << "\n";
            }
            file << "\n\n\n";
        }
        file.close();
    }
    
    void setDefaultBackgroundValue(openvdb::FloatGrid::Ptr& grid_pointer) {
        for (openvdb::FloatGrid::ValueOffIter iter = grid_pointer->beginValueOff(); iter; ++iter) {
            if(iter.getValue() < 0) iter.setValue(-1.0);
            else iter.setValue(1.0);
        }
    }
}

#endif