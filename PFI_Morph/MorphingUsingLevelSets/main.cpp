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

#include <iostream>
#include <fstream>
#include <iomanip> // setprecision
#include <sstream> // stringstream

template<class T1, class T2> void display(const T1, const T2);
openvdb::GridBase::Ptr readFile(const std::string);
void writeToFile(const std::string, openvdb::FloatGrid::Ptr);


double measureGrid(const openvdb::FloatGrid::Ptr);
void adjustScale(const openvdb::FloatGrid::Ptr&, const openvdb::FloatGrid::Ptr&);
void calcCentroid(const openvdb::FloatGrid::Ptr&);
void orientGrid(const openvdb::FloatGrid::Ptr&, const std::vector<openvdb::Vec3d>, openvdb::Vec3d);
double morphModels(openvdb::FloatGrid::Ptr&, openvdb::FloatGrid::Ptr&, double, std::string, std::string, std::string, std::string, std::string&);
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

openvdb::FloatGrid::Ptr getPlatonicVolume(int num_faces, float scale, float voxel_size, float half_width) {
    return openvdb::tools::createLevelSetPlatonic<openvdb::FloatGrid>(num_faces, scale, openvdb::Vec3f(0.0f), voxel_size, half_width);
}

openvdb::FloatGrid::Ptr getSphereVolume(float radius, float voxel_size, float half_width) {
    return openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(radius, openvdb::Vec3f(0.0f), voxel_size, half_width);
}

openvdb::FloatGrid::Ptr getCubeVolume(float scale, float voxel_size, float half_width) {
    return openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(scale, openvdb::Vec3f(0.0f), voxel_size, half_width);
}

void runExperiments() {
    openvdb::FloatGrid::Ptr source_grid1 = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(1, openvdb::Vec3f(0.0f), 0.01, 3);
    openvdb::FloatGrid::Accessor acc = source_grid1->getAccessor();
    double value;
    double bg = source_grid1->background();
    
    openvdb::Coord xyz;
    
    int min_index = -100, max_index = 100;
    
    for(int i = -100; i < 100; i++)
        for(int j = -100; j < 0; j++)
            for(int k = -100; k < 100; k++) {
                xyz = openvdb::Coord(i, j, k);
                
                acc.setValueOn(xyz, bg);
                //value = acc.getValue(xyz);
            }
    
    
    writeToFile("grid.vdb", source_grid1);
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

int main()
{
    openvdb::initialize();
  
    //openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("openvdb_cube.vdb"));
    
    
    //apparently, the greater the value of LEVEL_SET_HALF_WIDTH, the more its possible to scale
    //static const openvdb::Real LEVEL_SET_HALF_WIDTH = 3.0;
    
    
    
    //runExperiments();
    
//    openvdb::FloatGrid::Ptr source_grid3 = getSphereVolume(1, 0.03, 3);
//    openvdb::FloatGrid::Ptr target_grid3 = getPlatonicVolume(20, calcPlatonicScale(20, 1), 0.03, 10);
//    writeToFile("source_grid.vdb", source_grid3);
//    writeToFile("target_grid.vdb", target_grid3);
//    return 0;
    
    //KEEP TARGET NARROW BAND WIDE!
    
    float source_half_width = 3.0,
    target_half_width = 10.0,
    sphere_radius = 6.0;
    //scale = 5.0;
    
    int faces_arr[5] = {4, 6, 8, 12, 20};
//    int faces_arr[1] = {20};
    
//    float voxel_size_arr[4] = {0.3, 0.1, 0.07, 0.03};
//    std::string voxel_size_str_arr[4] = {"0.3", "0.1", "0.07", "0.03"};
    float voxel_size_arr[1] = {0.03};
    std::string voxel_size_str_arr[1] = {"0.03"};
    
//    float dt_array[3] = {1.0, 0.7, 0.3};
//    std::string dt_arr_str[3] = {"1.0", "0.7", "0.3"};
    float dt_array[1] = {0.25};
    std::string dt_arr_str[1] = {"0.25"};

    
    //float dt = 1.0,
    //voxel_size = 0.3;
    
    //std::string dt_str = "0.3",
    //            voxel_size_str = "0.03";
    
    std::ofstream file;
    std::string table = "";
    
    for(int k = 0; k < sizeof(dt_array)/sizeof(dt_array[0]); k++) {                     //dt
        for(int j = 0; j < sizeof(voxel_size_arr)/sizeof(voxel_size_arr[0]); j++) {     //voxel size
            
            file.open ("log.txt");
            file << "dt - " << dt_arr_str[k] << "\n";
            file << "Voxel size - " << voxel_size_str_arr[j] << "\n";
            file << "Source hw - " << source_half_width << "\n";
            file << "Target hw - " << target_half_width << "\n";
            
            file << "Sphere radius - " << sphere_radius << "\n";
            
            file.close();
            
            openvdb::FloatGrid::Ptr source_grid1 = nullptr;
            openvdb::FloatGrid::Ptr target_grid1 = nullptr;
            openvdb::FloatGrid::Ptr source_grid2 = nullptr;
            openvdb::FloatGrid::Ptr target_grid2 = nullptr;
            
            table += "<tr><td colspan='3'>dt = " + dt_arr_str[k] + ", Voxel size = " + voxel_size_str_arr[j] + "</td></tr>";
            
            
            
            for(int i = 0; i < sizeof(faces_arr)/sizeof(faces_arr[0]); i++) {           //faces arr
                file.open ("log.txt", std::ios_base::app);
                file << "Platonic scale - " << calcPlatonicScale(faces_arr[i], sphere_radius) << "\n\n";
                file.close();
                
                source_grid1 = getSphereVolume(sphere_radius, voxel_size_arr[j], source_half_width);
                target_grid1 = getPlatonicVolume(faces_arr[i], calcPlatonicScale(faces_arr[i], sphere_radius), voxel_size_arr[j], target_half_width);
                double total_energy1 = morphModels(source_grid1, target_grid1, dt_array[k],
                                                   "Sphere to " + std::to_string(faces_arr[i]) + " faces",
                                                   "s" + std::to_string(faces_arr[i]),
                                                   voxel_size_str_arr[j],
                                                   dt_arr_str[k],
                                                   table);
                
                table += "<td></td></tr>";
                source_grid2 = getPlatonicVolume(faces_arr[i], calcPlatonicScale(faces_arr[i], sphere_radius), voxel_size_arr[j], source_half_width);
                target_grid2 = getSphereVolume(sphere_radius, voxel_size_arr[j], target_half_width);
                double total_energy2 = morphModels(source_grid2, target_grid2, dt_array[k],
                                                   std::to_string(faces_arr[i]) + " faces to Sphere",
                                                   std::to_string(faces_arr[i]) + "s",
                                                   voxel_size_str_arr[j],
                                                   dt_arr_str[k],
                                                   table);
                
                file.open ("log.txt", std::ios_base::app);
                file << "Mean energy - " << (total_energy1 + total_energy2) / 2 << "\n\n\n\n";
                file.close();
                
                table += "<td>" + std::to_string((total_energy1 + total_energy2) / 2) + "</td></tr>";
                
                source_grid1 = nullptr;
                target_grid1 = nullptr;
                source_grid2 = nullptr;
                target_grid2 = nullptr;
            }
            file.open ("log.txt", std::ios_base::app);
            file << table + "\n\n\n\n";
        }
    }
    
    
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

bool checkStopMorph(const openvdb::FloatGrid::Ptr& source_grid, const openvdb::FloatGrid::Ptr& target_grid) {
    double threshold = 0.01, max_diff = 0.0, curr_diff;
    openvdb::Vec3d coord1, coord2;
    openvdb::FloatGrid::Accessor target_acc = target_grid->getAccessor();
    
    
    
    for (openvdb::FloatGrid::ValueOnIter iter = source_grid->beginValueOn(); iter; ++iter) {
        if(iter.getValue() < 0) {
            if(checkIfSurface(iter, source_grid)) {
//                curr_diff = openvdb::math::Abs(iter.getValue() - target_acc.getValue(iter.getCoord()));
//                coord1 = source_grid->indexToWorld(iter.getCoord());
//                coord2 = target_grid->indexToWorld(iter.getCoord());
//                distance = openvdb::math::Sqrt(openvdb::math::Pow(coord1.x() - coord2.x(), 2) + openvdb::math::Pow(coord1.y() - coord2.y(), 2) + openvdb::math::Pow(coord1.z() - coord2.z(), 2));
                curr_diff = openvdb::math::Abs(iter.getValue() - target_acc.getValue(iter.getCoord()));
                if(curr_diff > max_diff) max_diff = curr_diff;
            }
        }
    }
    std::cout << "Max diff - " << max_diff << std::endl;
    return max_diff < threshold;
}

double morphModels(openvdb::FloatGrid::Ptr& source_grid, openvdb::FloatGrid::Ptr& target_grid, double dt, std::string title, std::string dir, std::string voxel_size, std::string dt_str, std::string& table) {
    //adjustScale(source_grid, target_grid);
    const openvdb::math::Transform &source_trans = source_grid->transform();
    
    openvdb::util::NullInterrupter* interrupt = nullptr;
    openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
    
    size_t CFL_count = 0.0;
    double time = 0.0, time_inc = dt, energy_consumed = 0.0, total_energy = 0.0, threshold = 0.00001;
//    double prev_max = 0, curr_max = 0;
//    int equal_count = 0;
    
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
        
        CFL_count += ls_morph.advect(time, time + time_inc);
        
        //source_grid has been updated after advection
        const openvdb::math::Transform &source_trans_updt = source_grid->transform();
        computeMeanCurvature(source_trans_updt, source_grid, target_coord_meancurv);
        
        energy_consumed = computeMeanSumOfCurvatureDifferences(source_coord_meancurv, target_coord_meancurv);
        std::cout << energy_consumed << std::endl;
        total_energy += energy_consumed;
        
        file_name = "output/r6sx/dt" + dt_str + "/" + dir + "/" + voxel_size + "/advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
        writeToFile(file_name, source_grid);
        
        display("CFL iterations", CFL_count);
        display("File created", file_name);
        display("Energy", energy_consumed);
        display("Total", total_energy);
        std::cout << std::endl;
        
        time += time_inc;
        
        if(checkStopMorph(source_grid, target_grid)) break;
//        prev_max = curr_max;
//        curr_max = checkStopMorph(source_grid, target_grid);
//        if(prev_max == curr_max) {
//            if(++equal_count == 5) break;
//        }
//        else equal_count = 0;
        
    }
    
    //display("Energy used", total_energy);
    std::ofstream file;
    file.open ("log.txt", std::ios_base::app);
    file << title + "\n";
    file << "Iterations - " << CFL_count << "\n";
    file << "Total energy consumed - " << total_energy << "\n";
    file << "\n";
    file.close();
    
    //table += "<tr><td colspan='3'>" + title + "</td></tr>";
    table += "<tr><td>" + std::to_string(CFL_count) + "</td>";
    table += "<td>" + std::to_string(total_energy) + "</td>";
    
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