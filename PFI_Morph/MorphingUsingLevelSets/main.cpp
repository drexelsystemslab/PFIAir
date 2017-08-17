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

template<class T1, class T2> void display(const T1, const T2);
openvdb::GridBase::Ptr readFile(const std::string);
void writeToFile(const std::string, openvdb::FloatGrid::Ptr);


double measureGrid(const openvdb::FloatGrid::Ptr);
void adjustScale(const openvdb::FloatGrid::Ptr&, const openvdb::FloatGrid::Ptr&);
void calcCentroid(const openvdb::FloatGrid::Ptr&);
void orientGrid(const openvdb::FloatGrid::Ptr&, const std::vector<openvdb::Vec3d>, openvdb::Vec3d);
double morphModels(openvdb::FloatGrid::Ptr&, openvdb::FloatGrid::Ptr&);
bool checkIfSurface(openvdb::FloatGrid::ValueOnIter, const openvdb::FloatGrid::Ptr);
void computeMeanCurvature(const openvdb::math::Transform, const openvdb::FloatGrid::Ptr, std::map<openvdb::Coord, double>&);
double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>&, std::map<openvdb::Coord, double>&);

void testingOnOffVoxels(openvdb::FloatGrid::Ptr&);


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

void writeOnlySurface(const openvdb::FloatGrid::Ptr grid_pointer) {
    double bg = grid_pointer->background();
    int surface = 0, non_surface = 0;
    std::cout << grid_pointer->activeVoxelCount() << std::endl;
    for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
        if(iter.getValue() < 0) {
            if(!checkIfSurface(iter, grid_pointer)) {
                ++non_surface;
                iter.setValue(bg);
            }
            else ++surface;
        }
    }
    std::cout << surface << std::endl;
    std::cout << non_surface << std::endl;
    writeToFile("only_surface.vdb", grid_pointer);
}

int main()
{
    openvdb::initialize();
    
    //    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    
    //    openvdb::FloatGrid::Ptr source_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    //    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
    
    //    openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("openvdb_cube.vdb"));
    //    openvdb::FloatGrid::Ptr target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("openvdb_sphere.vdb"));
    
    //    openvdb::FloatGrid::Ptr source_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    //    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetIcosahedron<openvdb::FloatGrid>(1.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
    
    //    openvdb::FloatGrid::Ptr test_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("openvdb_sphere.vdb"));
    
    //    openvdb::FloatGrid::Ptr source_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(20.0f), 0.3f);
    //    openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("dragon_scaled.vdb"));
    //    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
    
    
    //    openvdb::FloatGrid::Ptr test_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("openvdb_sphere.vdb"));
    //    testingOnOffVoxels(test_grid);
    
    
    
    //apparently, the greater the value of LEVEL_SET_HALF_WIDTH, the more its possible to scale
    static const openvdb::Real LEVEL_SET_HALF_WIDTH = 3.0;
    
    //source should be narrow band level set
    openvdb::FloatGrid::Ptr source_grid1 = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.1f);
    //target should be full distance volume??
    openvdb::FloatGrid::Ptr target_grid1 = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.1f, float(LEVEL_SET_HALF_WIDTH));
    double total_energy1 = morphModels(source_grid1, target_grid1);
    display("Energy 1", total_energy1);
    
    
    //    //source should be narrow band level set
    //    openvdb::FloatGrid::Ptr source_grid2 = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
    //    //target should be full distance volume??
    //    openvdb::FloatGrid::Ptr target_grid2 = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    //    double total_energy2 = morphModels(source_grid2, target_grid2);
    
    
    //    display("Energy 1", total_energy1);
    //    display("Energy 2", total_energy2);
    //    display("Mean energy", (total_energy1 + total_energy2) / 2);
    
    return 0;
}

template<class T1, class T2>
void display(const T1 tag, const T2 msg) {
    tag != "" ? std::cout << tag << " - " << msg << std::endl : std::cout << msg << std::endl;
}

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

double checkStopMorph(const openvdb::FloatGrid::Ptr& source_grid, const openvdb::FloatGrid::Ptr& target_grid) {
    double threshold = 0.02, max_diff = 0.0, curr_diff;
    openvdb::Coord curr_coord;
    openvdb::FloatGrid::Accessor target_acc = target_grid->getAccessor();
    
    for (openvdb::FloatGrid::ValueOnIter iter = source_grid->beginValueOn(); iter; ++iter) {
        if(iter.getValue() < 0) {
            if(checkIfSurface(iter, source_grid)) {
                curr_diff = openvdb::math::Abs(iter.getValue() - target_acc.getValue(iter.getCoord()));
                if(curr_diff > max_diff) max_diff = curr_diff;
            }
        }
    }
    std::cout << "Max diff: " << max_diff << std::endl;
    return max_diff;
}

double morphModels(openvdb::FloatGrid::Ptr& source_grid, openvdb::FloatGrid::Ptr& target_grid) {
    adjustScale(source_grid, target_grid);
    const openvdb::math::Transform &source_trans = source_grid->transform();
    
    openvdb::util::NullInterrupter* interrupt = nullptr;
    openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
    
    size_t CFL_count;
    double time = 0.0, time_inc = 1.0, energy_consumed = 0.0, total_energy = 0.0, threshold = 0.00001;
    double prev_max = 0, curr_max = 0;
    std::string file_name = "";
    
    std::map<openvdb::Coord, double> source_coord_meancurv, target_coord_meancurv;
    
    computeMeanCurvature(source_trans, source_grid, source_coord_meancurv);
    
    
    while(true) {
        if(time != 0.0) {
            source_coord_meancurv = target_coord_meancurv;
            
            //std::cout << "Energy diff: " << prev_energy - energy_consumed << std::endl;
            
            //            if(openvdb::math::Abs(energy_consumed) < threshold)
            //                break;
            //
            //prev_energy = energy_consumed;
        }
        
        CFL_count = ls_morph.advect(time, time + time_inc);
        
        //source_grid has been updated after advection
        const openvdb::math::Transform &source_trans_updt = source_grid->transform();
        computeMeanCurvature(source_trans_updt, source_grid, target_coord_meancurv);
        
        energy_consumed = computeMeanSumOfCurvatureDifferences(source_coord_meancurv, target_coord_meancurv);
        std::cout << energy_consumed << std::endl;
        total_energy += energy_consumed;
        
        file_name = "advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
        writeToFile(file_name, source_grid);
        
        display("CFL iterations", CFL_count);
        display("File created", file_name);
        display("Energy", energy_consumed);
        display("Total", total_energy);
        std::cout << std::endl;
        
        time += time_inc;
        
        prev_max = curr_max;
        curr_max = checkStopMorph(source_grid, target_grid);
        if(prev_max == curr_max) break;
    }
    
    //display("Energy used", total_energy);
    return total_energy;
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

void computeMeanCurvature(const openvdb::math::Transform grid_transform, const openvdb::FloatGrid::Ptr grid_pointer, std::map<openvdb::Coord, double>& coord_meancurv) {
    double alpha, beta;
    openvdb::math::MeanCurvature<openvdb::math::MapBase, openvdb::math::DDScheme::CD_SECOND, openvdb::math::DScheme::CD_2ND> mc_obj;
    
    for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
        if(iter.getValue() < 0) {
            if(checkIfSurface(iter, grid_pointer)) {
                mc_obj.compute(*grid_transform.baseMap(), grid_pointer->getAccessor(), iter.getCoord(), alpha, beta);
                
                if(beta != 0.0)
                    coord_meancurv[iter.getCoord()] = alpha / beta;
                else
                    std::cout << "Found zero denominator (beta val)";
            }
        }
    }
}

double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>& source_coord_meancurv, std::map<openvdb::Coord, double>& target_coord_meancurv) {
    try {
        double sum_of_differences = 0.0;
        
        for (std::map<openvdb::Coord, double>::iterator it = source_coord_meancurv.begin(); it != source_coord_meancurv.end(); ++it) {
            //if terniary operator is not performed, target coords count is increased to accomodate default value
            sum_of_differences += openvdb::math::Abs((target_coord_meancurv.count(it->first) > 0 ? target_coord_meancurv[it->first] : 0) - source_coord_meancurv[it->first]);
        }
        return sum_of_differences / source_coord_meancurv.size();
    }
    catch(...) {
        std::cout << "Error" << std::endl;
        return 0;
    }
}


void testingOnOffVoxels(openvdb::FloatGrid::Ptr& grid_pointer) {
    double bg = grid_pointer->background();
    openvdb::Vec3d voxel_size = grid_pointer->voxelSize();
    
    int count = 0, less = 0, more = 0;
    
    count = 0, less = 0, more = 0;
    for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
        ++count;
        if(iter.getValue() < voxel_size[0] + 0.1) ++less;
        else ++more;
    }
    display("count on", count);
    display("less", less);
    display("more", more);
    
    
    
    for (openvdb::FloatGrid::ValueAllIter iter = grid_pointer->beginValueAll(); iter; ++iter) {
        ++count;
        if(iter.getValue() < -0.3 ) {
            ++less;
            iter.setValue(bg);
        }
        else ++more;
    }
    display("count all", count);
    display("less", less);
    display("more", more);
    
    
    
    count = 0, less = 0, more = 0;
    for (openvdb::FloatGrid::ValueOffIter iter = grid_pointer->beginValueOff(); iter; ++iter) {
        ++count;
        if(iter.getValue() < 0) ++less;
        else ++more;
    }
    display("count off", count);
    display("less", less);
    display("more", more);
    
    count = 0, less = 0, more = 0;
    for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
        ++count;
        if(iter.getValue() < 0) ++less;
        else ++more;
    }
    display("count on", count);
    display("less", less);
    display("more", more);
    
    writeToFile("settobackround.vdb", grid_pointer);
}