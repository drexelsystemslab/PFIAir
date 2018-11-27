#ifndef GRIDOPERATIONS_H
#define GRIDOPERATIONS_H

#include <openvdb/util/NullInterrupter.h>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <fstream>
#include <openvdb/tools/LevelSetFilter.h>

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
    void writeToFile(const std::string, const openvdb::FloatGrid::Ptr);
    void writeOnlySurface(const openvdb::FloatGrid::Ptr);
    bool checkIfSurface(openvdb::FloatGrid::ValueOnIter, const openvdb::FloatGrid::Ptr);
    
    double measureGrid(const openvdb::FloatGrid::Ptr);
    double calcPlatonicScale(int, double);
    
    void writeActiveVoxelVals();
    void setDefaultBackgroundValue(openvdb::FloatGrid::Ptr&);
    
    
    /**
     * Read in a VDB file
     */
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
    
    /**
     * Write a VDB model to a vdb file
     */
    void writeToFile(const std::string file_name, const openvdb::FloatGrid::Ptr grid_pointer) {
        grid_pointer->setGridClass(openvdb::GRID_LEVEL_SET);
        openvdb::io::File file(file_name);
        openvdb::GridPtrVec grids;
        grids.push_back(grid_pointer);
        file.write(grids);
        file.close();
    }
    
    /**
     * Write only the surface voxels
     * NOTE: This method is currently unused
     */
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
    
    /**
     * Count up the surface voxels by iteration
     */
    int countSurfaceVoxels(const openvdb::FloatGrid::Ptr grid_pointer) {
        int count = 0;
        for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0) {
                if(GridOperations::checkIfSurface(iter, grid_pointer)) {
                    count++;
                }
            }
        }
        return count;
    } 
    
    /**
     * Check if a voxel is on the surface of the grid
     */
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
    
    /**
     * Measure the surface area of the grid in world units
     *
     * NOTE: openvdb::tools::LevelSetMeasure has a flag to count voxels
     * instead of world units. I am not sure if this would be faster
     * than the calculation in GridOperations::countSurfaceVoxels().
     */
    double measureGrid(const openvdb::FloatGrid::Ptr grid_pointer) {
        openvdb::util::NullInterrupter* interrupt = nullptr;
        openvdb::tools::LevelSetMeasure <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_measure(*(grid_pointer), interrupt);
        
        double area = 0.0, volume = 0.0;
        ls_measure.measure(area, volume);
        
        return area;
    }
    
    /**
     * This is only used in Tests.h
     */
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
    
    /**
     * This is also unused
     */
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
    
    /**
     * Use OpenVDB to peform a morphological opening
     */
    void performMorphologicalOpening(
            openvdb::FloatGrid::Ptr& grid_pointer, 
            int norm_count=5,
            int offset_voxels=5) {
        openvdb::tools::LevelSetFilter<openvdb::FloatGrid> lsf(*(grid_pointer));
        double voxel_size = grid_pointer->voxelSize()[0];
        // Set number of normalizations
        lsf.setNormCount(norm_count);
        
        // Offset by one voxel
        lsf.offset(offset_voxels * voxel_size);
        lsf.normalize();
        
        // Offset by one voxel in the other direction.
        lsf.offset(-offset_voxels * voxel_size);
        lsf.normalize();
    }
    
    void getWorldCoordinates(openvdb::FloatGrid::Ptr grid, Eigen::MatrixXd& mat) {
        openvdb::Coord curr_coord;
        int counter = 0;
        for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter) {
            if(iter.getValue() < 0) {
                if(GridOperations::checkIfSurface(iter, grid)) {
                    curr_coord = iter.getCoord();
                    mat(0, counter) = curr_coord.x();
                    mat(1, counter) = curr_coord.y();
                    mat(2, counter) = curr_coord.z();
                    mat(3, counter) = 1.0;
                    counter++;
                }
            }
        }
    }
}

#endif





















