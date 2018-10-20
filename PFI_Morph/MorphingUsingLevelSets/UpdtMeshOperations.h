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
    // Why are these implemented in the same h files?
    void calcBoundingBox(const Eigen::MatrixXd, openvdb::Vec3d&, std::vector<double>&s, bool includePCA = false);
    void normalizeHomogenousCoords(Eigen::MatrixXd& vertices);
    Eigen::Matrix4d getTranslateMatrix(openvdb::Vec3d);
    Eigen::Matrix4d getScaleMatrix(double);
    void normalizeHomogenousCoords(Eigen::MatrixXd&);
    void convertMeshToVolume(std::string, std::string, std::string, float, double);
    
    /**
     * Read a .obj model into a list of vectors
     * TODO: This should create a class object instead of modifying vectors
     * directly.
     *
     * v_list - list of vertex coordinates
     * vn_list - list of vertex normals
     * f_list - list of face elements (list of vertex indices)
     */
    void readOBJ(
            std::string filepath, 
            std::vector<std::vector<std::string>>& v_list, 
            std::vector<std::vector<std::string>>& vn_list, 
            std::vector<std::vector<std::string>>& f_list) {
        
        std::ifstream infile(filepath);
        std::string line;
        
        std::vector<std::string> items;
        
        
        while (std::getline(infile, line))
        {
            // Split the line on space
            items = CommonOperations::prep_do_split(line, ' ');
            
            if(items.size()) {
                // Add a vertex 
                if(items[0] == "v") {
                    v_list.push_back(items);
                // Add a vertex normal
                } else if(items[0] == "vn") {
                    vn_list.push_back(items);
                // Face elements (vertex index list)
                } else if(items[0] == "f") {
                    f_list.push_back(items);
                } else {
                    // Ignore any other type of line
                }
            }
        }
    }
    
    /**
     * Write a .obj file to disk
     *
     * filename - the output file
     * vertices - matrix of vertices
     * f_list - face elements (vertex indices)
     * include_axis - set this true to include PCA axis
     */
    void writeOBJ(
        std::string filename, 
        Eigen::MatrixXd vertices, 
        std::vector<std::vector<std::string>>& f_list, 
        bool include_axis) {

        std::ofstream file;
        file.open (filename);
        
        int vertices_size = vertices.cols();
        
        // Write the vertices
        for(int i = 0; i < vertices_size; i++) {
            file << "v " << vertices(0, i) << " " << vertices(1, i) << " " << vertices(2, i) << "\n";
        }
        
        // Write the face comonents
        for(int i = 0; i < f_list.size(); i++) {
            file << "f " << f_list[i][2] << " " << f_list[i][3] << " " << f_list[i][4] << "\n";
        }
        
        // List face elements with normals
        // The last 6 columns are PCA axis information.
        if(include_axis) {
            
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
    
    /**
     * Compute the centroid of a set of vertices
     */
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
    
    /**
     * Turn a set of vectors into a matrix.
     */
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
    
    /**
     * Perform principal component analysis 
     */
    void performPCA(Eigen::MatrixXd& all_vertices, Eigen::Matrix4d& rot_mat) {
        
        // Compute the covariance matrix of vertices
        // X * X^T
        Eigen::Matrix3d covariance_mat = (
            all_vertices.block(0, 0, 3, all_vertices.cols()) *
            all_vertices.block(0, 0, 3, all_vertices.cols()).transpose());

        // Compute the eigenvalues of the covariance matrix
        Eigen::EigenSolver<Eigen::Matrix3d> es(covariance_mat);
        Eigen::Matrix<double, 3, 1> eigen_vals = es.eigenvalues().col(0).real();
        Eigen::Matrix<double, 3, 3> eigen_vecs = es.eigenvectors().real();
        
        // Get the real components of the eigenvalues
        std::vector<double> real_vals = { 
            eigen_vals(0), 
            eigen_vals(1), 
            eigen_vals(2) 
        };
        
        /**
         * Find the max eigenvalue
         * TODO: Maybe make a custom min and max function instead of this?
         */
        int max = real_vals[0];
        int max_index = 0;
        int mid_index = 0;
        int min_index = 0;
        for(int i = 1 ; i < real_vals.size(); i++) {
            if(real_vals[i] > max) {
                max = real_vals[i];
                max_index = i;
            }
        }

        // Update the minimum index given the maximum.
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
        
        // Compute the axis vectors
        Eigen::Matrix<double, 3, 1> z_axis_vec = eigen_vecs.col(max_index);
        Eigen::Matrix<double, 3, 1> y_axis_vec = eigen_vecs.col(mid_index);
        Eigen::Matrix<double, 3, 1> x_axis_vec = eigen_vecs.col(min_index);
        
        // Create the rotation matrix based on eigenvectors
        rot_mat <<  x_axis_vec(0), x_axis_vec(1), x_axis_vec(2), 0,
        y_axis_vec(0), y_axis_vec(1), y_axis_vec(2), 0,
        z_axis_vec(0), z_axis_vec(1), z_axis_vec(2), 0,
        0, 0, 0, 1;
    }
    
    /**
     * Calculate an axis-aligned bounding box of the model
     */
    void calcBoundingBox(
            const Eigen::MatrixXd vertices, 
            openvdb::Vec3d& center, 
            std::vector<double>& axis_lengths, 
            bool includePCA) {


        const double limit = 10.0e10; //openvdb::math::Pow(10, 10);
        double min_x = limit;
        double max_x = -limit;
        double min_y = limit;
        double max_y = -limit;
        double min_z = limit;
        double max_z = -limit;
        double x; 
        double y;
        double z;
        
        // Ignore PCA columns
        int size = vertices.cols();
        if(includePCA) size -= 6;
        
        // Find the extrema
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
        
        // Compute the length of each side of the bounding box.
        axis_lengths[0] = fabs(max_x - min_x);
        axis_lengths[1] = fabs(max_y - min_y);
        axis_lengths[2] = fabs(max_z - min_z);
        
        // The center of the bounding box is the
        // midpoint of the min/max.
        center[0] = (min_x + max_x) / 2;
        center[1] = (min_y + max_y) / 2;
        center[2] = (min_z + max_z) / 2;
        
        //        max_z_val = openvdb::math::Max(openvdb::math::Abs(min_z), openvdb::math::Abs(max_z));
    }
    
    /**
     * Construct an INVERSE scaling matrix.
     */
    Eigen::Matrix4d getScaleMatrix(double d) {
        Eigen::Matrix4d scale_mat;
        scale_mat <<
        1/d, 0, 0, 0,
        0, 1/d, 0, 0,
        0, 0, 1/d, 0,
        0, 0, 0, 1;
        return scale_mat;
    }
    
    /**
     * Construct an INVERSE translation matrix
     * TODO: See Eigen's Geometry module for an easier method of 
     * getting transformation matrices
     */
    Eigen::Matrix4d getTranslateMatrix(openvdb::Vec3d center) {
        Eigen::Matrix4d translate_mat;
        translate_mat <<
        1, 0, 0, -center.x(),
        0, 1, 0, -center.y(),
        0, 0, 1, -center.z(),
        0, 0, 0, 1;
        return translate_mat;
    }
    
    /**
     * Normalize homogeneous coordinates by dividing by the w component.
     * TODO: Eigen supports division by a scalar.
     */
    void normalizeHomogenousCoords(Eigen::MatrixXd& vertices) {
        for(int i = 0; i < vertices.cols(); i++) {
            vertices(0, i) /= vertices(3, i);
            vertices(1, i) /= vertices(3, i);
            vertices(2, i) /= vertices(3, i);
            vertices(3, i) /= vertices(3, i);
        }
    }
    
    /**
     * Convert an OBJ file to a VDB file
     */
    void convertMeshToVolume(
            std::string obj_filename, 
            std::string vdb_filename, 
            std::string write_path, 
            float bandwidth, 
            double voxel_size) {
        
        // Wrapper for the model
        PFIAir::Container model = PFIAir::Container();
        
        // Load the mesh and compute its center point
        model.loadMeshModel(obj_filename);
        model.computeMeshCenter();
        
        // pre-scale the model
        model.setScale(openvdb::Vec3d(voxel_size));
        
        // Make a level set from the model
        auto shape = model.getWaterTightLevelSetWithBandWidth(bandwidth);
        
        //Export the model to a VDB file
        model.exportModel(write_path + vdb_filename, shape);
    }
    
    /**
     * Add the PCA axes to the list of vertices
     */
    void addPCAAxisToVertices(
            Eigen::MatrixXd& vertices1,
            openvdb::Vec3d center1, 
            Eigen::Matrix4d rot_mat1) {
 
        // Set one of the columnns to 0.
        // TODO: why the 7th to last column?
        vertices1(0, vertices1.cols() - 7) = 0.0;
        vertices1(1, vertices1.cols() - 7) = 0.0;
        vertices1(2, vertices1.cols() - 7) = 0.0;
        
        // Store PCA axis into the vertex list.
        // TODO: Why are there two vectors per vertex shifted 0.01 apart in
        // the y-direction?
        for(int i = 0; i < 3; i++) {
            // TODO: Look into Eigen blocks for submatrix assignment.
            // TODO: what are all these magic nnumbers doing? 
            vertices1(0, vertices1.cols() - ((3 - i) * 2)) = center1.x() + rot_mat1(2 - i, 0);
            vertices1(1, vertices1.cols() - ((3 - i) * 2)) = center1.y() + rot_mat1(2 - i, 1);
            vertices1(2, vertices1.cols() - ((3 - i) * 2)) = center1.z() + rot_mat1(2 - i, 2);
            vertices1(3, vertices1.cols() - ((3 - i) * 2)) = 1.0;
            
            // TODO: Why is 0.01 added to the y component?
            vertices1(0, vertices1.cols() - ((3 - i) * 2) + 1) = center1.x() + rot_mat1(2 - i, 0);
            vertices1(1, vertices1.cols() - ((3 - i) * 2) + 1) = center1.y() + rot_mat1(2 - i, 1) + 0.01;
            vertices1(2, vertices1.cols() - ((3 - i) * 2) + 1) = center1.z() + rot_mat1(2 - i, 2);
            vertices1(3, vertices1.cols() - ((3 - i) * 2) + 1) = 1.0;
        }
    }
    
    
    /**
     * Compute the skewness of the vertices
     */
    double calcSkewness(const Eigen::MatrixXd vertices, bool pcaIncluded) {
        if(vertices.cols() == 0) return 0;
        
        // Determine how many vertices we have depending
        // on whether we have the PCA axes included.
        int vertices_size = pcaIncluded ? vertices.cols() - 6 : vertices.cols();

        double z_values[vertices_size],
        z_total = 0.0;
        
        std::map<double, int> map_z_count; 
       
        // Compute the mean of the z values
        for(int i = 0; i < vertices_size; i++) {
            z_values[i] = vertices(2, i);
            z_total += vertices(2, i);
        }
        double mean = z_total / vertices_size;
        
        // Compute the median z values
        std::sort(z_values, z_values + vertices_size);
        double median;
        if(vertices_size % 2 == 0)
            median = (z_values[vertices_size/2] + z_values[vertices_size/2 - 1]) / 2;
        else 
            median = z_values[vertices_size/2];
        
        // Compute an estimation of the skewness. Technically skewness
        // should be divided by the standard deviation, but in this case
        // the sign of the skewness is most important.
        double skewness = mean - median;
        return skewness;
    }
    
    /**
     *Get an INVERSE rotation matrix around the x-axis
     */
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
    
    /**
     * Get an INVERSE rotation matrix around the y-axis
     */
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
    
    /**
     * Get an INVERSE rotation matrix around the z-axis
     */
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
    
    /**
     * Preprocess the mesh, translating it so its center is at the origin.
     * Also perform a morphological opening.
     */
    void coordsHandler(
            std::string output_dir,
            std::string filepath, 
            Eigen::MatrixXd& mesh_coords, 
            Eigen::MatrixXd& vdb_coords, 
            std::vector<std::vector<std::string>>& f_list, 
            bool includePCA) {
        std::vector<std::vector<std::string>> v_list;
        std::vector<std::vector<std::string>> vn_list;
        std::set<openvdb::Vec3d> vertices_set;
        openvdb::Vec3d centroid(0,0,0), center(0, 0, 0);
        
        // Read the input object file
        UpdtMeshOperations::readOBJ(filepath, v_list, vn_list, f_list);
        
        // Get the number of elements.
        size_t no_elem;
        if(v_list.size() > 0) no_elem = v_list[0].size();
        else return;
        
        //Convert the vertices into a matrix 
        mesh_coords.resize(4, v_list.size()); 
        for(int i = 0; i < v_list.size(); i++) {
            mesh_coords(0, i) = std::stod(v_list[i][no_elem - 3]);
            mesh_coords(1, i) = std::stod(v_list[i][no_elem - 2]);
            mesh_coords(2, i) = std::stod(v_list[i][no_elem - 1]);
            mesh_coords(3, i) = 1.0;
        }
        
        // Translate the mesh so the center is at the origin. 
        calcCentroid(mesh_coords, centroid);
        mesh_coords = getTranslateMatrix(centroid) * mesh_coords;
        
        // Using the bounding box, pick the maximum dimension
        std::vector<double> axis_lengths(3);
        calcBoundingBox(mesh_coords, center, axis_lengths);
        double max_length = openvdb::math::Max(
            axis_lengths[0], axis_lengths[1]);
        max_length = openvdb::math::Max(max_length, axis_lengths[2]);
        
        // If the principal axis is smaller than 1, scale it up
        if(max_length < 1.0) {
            mesh_coords = getScaleMatrix(max_length) * mesh_coords;
        }

        // Temporary model files
        // TODO: What does SRT stand for?
        std::string temp_obj = output_dir + "srt.obj";
        std::string temp_vdb = output_dir + "srt.vdb";
        
        // TODO: Do we need both files?
        // Store the results  to an OBJ file
        UpdtMeshOperations::writeOBJ(temp_obj, mesh_coords, f_list, includePCA);
        // Convert to a VDB file
        convertMeshToVolume(temp_obj, temp_vdb, "", 3, 0.05);
        
        // Read in the VDB file
        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(
            GridOperations::readFile(temp_vdb));

        // Perform a morphological opening on the level sets
        GridOperations::performMorphologicalOpening(grid);

        // Update the matrix
        vdb_coords.resize(4, GridOperations::countSurfaceVoxels(grid));

        //TODO: What does this do?
        GridOperations::getWorldCoordinates(grid, vdb_coords);
    }
    
    /**
     * Orient a mesh and write the results to disk in OBJ format
     *
     * filepath - Path to the original OBJ file
     * write_name - Path to the oriented OBJ file
     */
    void doAllMeshOperations(
            std::string output_dir,
            std::string filepath, 
            std::string write_name) {
        if(write_name == "test/.DS_Store") return;
        
        openvdb::Vec3d center(0, 0, 0), centroid(0, 0, 0);
        Eigen::MatrixXd mesh_coords, vdb_coords;
        Eigen::Matrix4d rot_mat;
        std::vector<std::vector<std::string>> f_list;
        std::vector<double> axis_lengths(3);
        bool includePCA = false;
        
        coordsHandler(output_dir, filepath, mesh_coords, vdb_coords, f_list, includePCA);

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
    
    /**
     * This method is not currently used.
     */
     /*
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
    */
}

#endif

