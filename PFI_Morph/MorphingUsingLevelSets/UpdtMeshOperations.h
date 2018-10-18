#ifndef UPDTMESHOPERATIONS_H
#define UPDTMESHOPERATIONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <dirent.h>
#include <algorithm>
#include <set>
#include "CommonOperations.h"
#include "Container.hpp"
#include "GridOperations.h"

namespace UpdtMeshOperations {
    void calcBoundingBox(const Eigen::MatrixXd, openvdb::Vec3d&, std::vector<double>&s, bool includePCA = false);
    void normalizeHomogenousCoords(Eigen::MatrixXd& vertices);
    Eigen::Matrix4d getTranslateMatrix(openvdb::Vec3d);
    Eigen::Matrix4d getScaleMatrix(double);
    void normalizeHomogenousCoords(Eigen::MatrixXd&);
    void convertMeshToVolume(std::string, std::string, std::string, float, double);
    
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
    
    void writeOBJ(std::string filename, Eigen::MatrixXd vertices, std::vector<std::vector<std::string>>& f_list, bool includeAxis) {
        std::ofstream file;
        file.open (filename);
        
        int vertices_size = vertices.cols();
        
        for(int i = 0; i < vertices_size; i++) {
            file << "v " << vertices(0, i) << " " << vertices(1, i) << " " << vertices(2, i) << "\n";
        }
        
        for(int i = 0; i < f_list.size(); i++) {
            file << "f " << f_list[i][2] << " " << f_list[i][3] << " " << f_list[i][4] << "\n";
        }
        
        if(includeAxis) {
            
            file << "f " << vertices_size - 6 << "//" << vertices_size - 6 << " "
            << vertices_size - 5 << "//" << vertices_size - 5 << " "
            << vertices_size - 4 << "//" << vertices_size - 4 << "\n";
            
            file << "f " << vertices_size - 6 << "//" << vertices_size - 6 << " "
            << vertices_size - 3 << "//" << vertices_size - 3 << " "
            << vertices_size - 2 << "//" << vertices_size - 2 << "\n";
            
            file << "f " << vertices_size - 6 << "//" << vertices_size - 6 << " "
            << vertices_size - 1 << "//" << vertices_size - 1 << " "
            << vertices_size << "//" << vertices_size << "\n";
        }
        
        file.close();
    }
    
    void calcCentroid(Eigen::MatrixXd& vertices, openvdb::Vec3d& centroid) {
        centroid.x() = 0; centroid.y() = 0; centroid.z() = 0;
        if(!vertices.size()) return;
        
        openvdb::Vec3d sum_coords(0, 0, 0), curr_coord;
        for(int i = 1; i < vertices.cols(); i++) {
            curr_coord.x() = vertices(0, i);
            curr_coord.y() = vertices(1, i);
            curr_coord.z() = vertices(2, i);
            
            sum_coords += curr_coord;
        }
        
        sum_coords /= vertices.cols();
        centroid[0] = sum_coords.x();
        centroid[1] = sum_coords.y();
        centroid[2] = sum_coords.z();
    }
    
    void getMatFromSet(std::set<openvdb::Vec3d>& curr_set, Eigen::MatrixXd& new_mat) {
        int i = 0;
        for (std::set<openvdb::Vec3d>::iterator iter= curr_set.begin(); iter!= curr_set.end(); ++iter) {
            new_mat(0, i) = (*iter).x();
            new_mat(1, i) = (*iter).y();
            new_mat(2, i) = (*iter).z();
            new_mat(3, i) = 1.0;
            i++;
        }
    }
    
    void performPCA(Eigen::MatrixXd& all_vertices, Eigen::Matrix4d& rot_mat) {
        
        Eigen::Matrix3d covariance_mat = all_vertices.block(0, 0, 3, all_vertices.cols()) *
                                         all_vertices.block(0, 0, 3, all_vertices.cols()).transpose();

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
        
        Eigen::Matrix<double, 3, 1> z_axis_vec = eigen_vecs.col(max_index);
        Eigen::Matrix<double, 3, 1> y_axis_vec = eigen_vecs.col(mid_index);
        Eigen::Matrix<double, 3, 1> x_axis_vec = eigen_vecs.col(min_index);
        
        rot_mat <<  x_axis_vec(0), x_axis_vec(1), x_axis_vec(2), 0,
        y_axis_vec(0), y_axis_vec(1), y_axis_vec(2), 0,
        z_axis_vec(0), z_axis_vec(1), z_axis_vec(2), 0,
        0, 0, 0, 1;
    }
    
    void calcBoundingBox(const Eigen::MatrixXd vertices, openvdb::Vec3d& center, std::vector<double>& axis_lengths, bool includePCA) {
        double limit = openvdb::math::Pow(10, 10);
        double min_x = limit, max_x = -limit, min_y = limit, max_y = -limit, min_z = limit, max_z = -limit, x, y, z;
        
        int size = vertices.cols();
        if(includePCA) size -= 6;
        
        for(int i = 0; i < size; i++) {
            x = vertices(0, i);
            y = vertices(1, i);
            z = vertices(2, i);
            
            if(x < min_x) min_x = x;
            else if(x > max_x) max_x = x;
            
            if(y < min_y) min_y = y;
            else if(y > max_y) max_y = y;
            
            if(z < min_z) min_z = z;
            else if(z > max_z) max_z = z;
            
        }
        
        axis_lengths[0] = fabs(max_x - min_x);
        axis_lengths[1] = fabs(max_y - min_y);
        axis_lengths[2] = fabs(max_z - min_z);
        
        center[0] = (min_x + max_x) / 2;
        center[1] = (min_y + max_y) / 2;
        center[2] = (min_z + max_z) / 2;
        
        //        max_z_val = openvdb::math::Max(openvdb::math::Abs(min_z), openvdb::math::Abs(max_z));
    }
    
    Eigen::Matrix4d getScaleMatrix(double d) {
        Eigen::Matrix4d scale_mat;
        scale_mat <<
        1/d, 0, 0, 0,
        0, 1/d, 0, 0,
        0, 0, 1/d, 0,
        0, 0, 0, 1;
        return scale_mat;
    }
    
    Eigen::Matrix4d getTranslateMatrix(openvdb::Vec3d center) {
        Eigen::Matrix4d translate_mat;
        translate_mat <<
        1, 0, 0, -center.x(),
        0, 1, 0, -center.y(),
        0, 0, 1, -center.z(),
        0, 0, 0, 1;
        return translate_mat;
    }
    
    void normalizeHomogenousCoords(Eigen::MatrixXd& vertices) {
        for(int i = 0; i < vertices.cols(); i++) {
            vertices(0, i) /= vertices(3, i);
            vertices(1, i) /= vertices(3, i);
            vertices(2, i) /= vertices(3, i);
            vertices(3, i) /= vertices(3, i);
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
    
    void addPCAAxisToVertices(Eigen::MatrixXd& vertices1, openvdb::Vec3d center1, Eigen::Matrix4d rot_mat1) {
 
        vertices1(0, vertices1.cols() - 7) = 0.0;
        vertices1(1, vertices1.cols() - 7) = 0.0;
        vertices1(2, vertices1.cols() - 7) = 0.0;
        
        for(int i = 0; i < 3; i++) {
            vertices1(0, vertices1.cols() - ((3 - i) * 2)) = center1.x() + rot_mat1(2 - i, 0);
            vertices1(1, vertices1.cols() - ((3 - i) * 2)) = center1.y() + rot_mat1(2 - i, 1);
            vertices1(2, vertices1.cols() - ((3 - i) * 2)) = center1.z() + rot_mat1(2 - i, 2);
            vertices1(3, vertices1.cols() - ((3 - i) * 2)) = 1.0;
            
            vertices1(0, vertices1.cols() - ((3 - i) * 2) + 1) = center1.x() + rot_mat1(2 - i, 0);
            vertices1(1, vertices1.cols() - ((3 - i) * 2) + 1) = center1.y() + rot_mat1(2 - i, 1) + 0.01;
            vertices1(2, vertices1.cols() - ((3 - i) * 2) + 1) = center1.z() + rot_mat1(2 - i, 2);
            vertices1(3, vertices1.cols() - ((3 - i) * 2) + 1) = 1.0;
        }
    }
    
    
    double calcSkewness(const Eigen::MatrixXd vertices, bool pcaIncluded) {
        if(vertices.cols() == 0) return 0;
        
        int vertices_size = pcaIncluded ? vertices.cols() - 6 : vertices.cols();
        double z_values[vertices_size],
        z_total = 0.0;
        
        std::map<double, int> map_z_count;
        
        
        for(int i = 0; i < vertices_size; i++) {
            z_values[i] = vertices(2, i);
            z_total += vertices(2, i);
        }
        double mean = z_total / vertices_size;
        
        std::sort(z_values, z_values + vertices_size);
        double median;
        if(vertices_size % 2 == 0) {
            median = (z_values[vertices_size/2] + z_values[vertices_size/2 - 1]) / 2;
        }
        else median = z_values[vertices_size/2];
        
        double skewness = mean - median;
        return skewness;
    }
    
    Eigen::Matrix4d getXAxisRotMat(double degrees) {
        
        double rads = degrees * (M_PI / 180);
        Eigen::Matrix4d x_rot_mat;
        
        x_rot_mat <<
        1, 0, 0, 0,
        0, cos(rads), sin(rads), 0,
        0, -sin(rads), cos(rads), 0,
        0, 0, 0, 1;
        
        return x_rot_mat;
    }
    
    Eigen::Matrix4d getYAxisRotMat(double degrees) {
        
        double rads = degrees * (M_PI / 180);
        Eigen::Matrix4d y_rot_mat;
        y_rot_mat <<
        cos(rads), 0, -sin(rads), 0,
        0, 1, 0, 0,
        sin(rads), 0, cos(rads), 0,
        0, 0, 0, 1;
        
        return y_rot_mat;
    }
    
    Eigen::Matrix4d getZAxisRotMat(double degrees) {
        
        double rads = degrees * (M_PI / 180);
        Eigen::Matrix4d z_rot_mat;
        
        z_rot_mat <<
        cos(rads), sin(rads), 0, 0,
        -sin(rads), cos(rads), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
        
        return z_rot_mat;
    }
    
    /*void doAllMeshOperationsAndDrawAxis(std::string filepath1, std::string write_name) {
        std::vector<std::vector<std::string>> v_list;
        std::vector<std::vector<std::string>> vn_list;
        std::vector<std::vector<std::string>> f_list1;
        openvdb::Vec3d center1(0, 0, 0), centroid(0, 0, 0);
        
        Eigen::Matrix4d rot_mat1;
        std::vector<double> axis_lengths1(3);
        
        UpdtMeshOperations::readOBJ(filepath1, v_list, vn_list, f_list1);
        Eigen::MatrixXd vertices1(4, v_list.size() + 7);
        UpdtMeshOperations::performPCA(v_list, vertices1, rot_mat1);
        
        calcCentroid(vertices1, centroid);
        UpdtMeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
        addPCAAxisToVertices(vertices1, centroid, rot_mat1);
        UpdtMeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
        
        vertices1 = rot_mat1 * getTranslateMatrix(centroid) * vertices1;
        UpdtMeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
        
        calcBoundingBox(vertices1, center1, axis_lengths1, true);
        vertices1.block(0, 0, 4, vertices1.cols() - 7) = getScaleMatrix(axis_lengths1[2]) * getTranslateMatrix(center1) *
                                                         vertices1.block(0, 0, 4, vertices1.cols() - 7);
        
        UpdtMeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
        if(calcSkewness(vertices1, true) > 0)
            vertices1 = getYAxisRotMat() * vertices1;
        
        UpdtMeshOperations::writeOBJ(write_name, vertices1, f_list1, true);
    }*/
    
    void coordsHandler(std::string filepath, Eigen::MatrixXd& mesh_coords, Eigen::MatrixXd& vdb_coords, std::vector<std::vector<std::string>>& f_list, bool includePCA) {
        std::vector<std::vector<std::string>> v_list;
        std::vector<std::vector<std::string>> vn_list;
        std::set<openvdb::Vec3d> vertices_set;
        openvdb::Vec3d centroid(0,0,0), center(0, 0, 0);
        
        UpdtMeshOperations::readOBJ(filepath, v_list, vn_list, f_list);
        
        size_t no_elem;
        if(v_list.size() > 0) no_elem = v_list[0].size();
        else return;
        
        mesh_coords.resize(4, v_list.size());
        
        for(int i = 0; i < v_list.size(); i++) {
            mesh_coords(0, i) = std::stod(v_list[i][no_elem - 3]);
            mesh_coords(1, i) = std::stod(v_list[i][no_elem - 2]);
            mesh_coords(2, i) = std::stod(v_list[i][no_elem - 1]);
            mesh_coords(3, i) = 1.0;
        }
        
        calcCentroid(mesh_coords, centroid);
        mesh_coords = getTranslateMatrix(centroid) * mesh_coords;
        
        std::vector<double> axis_lengths(3);
        calcBoundingBox(mesh_coords, center, axis_lengths);
        double max_length = openvdb::math::Max(axis_lengths[0], axis_lengths[1]);
        max_length = openvdb::math::Max(max_length, axis_lengths[2]);
        
        if(max_length < 1.0) {
            mesh_coords = getScaleMatrix(max_length) * mesh_coords;
        }
        
        UpdtMeshOperations::writeOBJ("srt.obj", mesh_coords, f_list, includePCA);
        
        convertMeshToVolume("srt.obj", "srt.vdb", "", 3, 0.05);
        
        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile("srt.vdb"));
        GridOperations::performMorphologicalOpening(grid);
        vdb_coords.resize(4, GridOperations::countSurfaceVoxels(grid));
        GridOperations::getWorldCoordinates(grid, vdb_coords);
    }
    
    /**
     *
     */
    void doAllMeshOperations(std::string filepath, std::string write_name) {
        if(write_name == "test/.DS_Store") return;
        
        openvdb::Vec3d center(0, 0, 0), centroid(0, 0, 0);
        Eigen::MatrixXd mesh_coords, vdb_coords;
        Eigen::Matrix4d rot_mat;
        std::vector<std::vector<std::string>> f_list;
        std::vector<double> axis_lengths(3);
        bool includePCA = false;
        
        coordsHandler(filepath, mesh_coords, vdb_coords, f_list, includePCA);

        // Perform PCA on the models to figure out how to orient
        // the model
        //TODO: Use just vdb_coords <-- ???
        UpdtMeshOperations::performPCA(vdb_coords, rot_mat);
        
        // Orient the model 
        mesh_coords = rot_mat * mesh_coords;

        // Calculate the new bounding box
        calcBoundingBox(mesh_coords, center, axis_lengths, includePCA);

        // Calculate the scale of thhe model
        mesh_coords = getScaleMatrix(axis_lengths[2]) * getTranslateMatrix(center) * mesh_coords;

        // Write the object to disk
        // TODO: why is this called twice?
        UpdtMeshOperations::writeOBJ(write_name, mesh_coords, f_list, includePCA);

        //If the skewness is positive, we need to rotate the model 180 degrees
        if(calcSkewness(vdb_coords, includePCA) > 0)
            mesh_coords = getYAxisRotMat(180) * mesh_coords;
        
        // Write the modified OBJ file to disk
        UpdtMeshOperations::writeOBJ(write_name, mesh_coords, f_list, includePCA);
        
    }
    
    void performActionForAllObjs() {
        DIR *dir1;
        struct dirent *ent1;
        std::string obj_path1 = "original_objs/", obj_path2 = "test/";
        
        if ((dir1 = opendir ("original_objs")) != NULL) {
            while ((ent1 = readdir (dir1)) != NULL) {
                if(ent1->d_type == DT_REG) {
                    UpdtMeshOperations::doAllMeshOperations(obj_path1 + ent1->d_name, obj_path2 + ent1->d_name);
                }
            }
        }
    }
}

#endif

