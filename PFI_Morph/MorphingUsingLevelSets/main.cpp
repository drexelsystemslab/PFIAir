#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetMorph.h>
#include <openvdb/tools/LevelSetMeasure.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/math/Maps.h>
#include <openvdb/math/Operators.h>
#include <openvdb/math/FiniteDifference.h>
#include <openvdb/util/NullInterrupter.h>
#include <vector>
#include <map>

template<class T1, class T2> void display(const T1, const T2);
void writeToFile(const std::string, openvdb::FloatGrid::Ptr);

double measureGrid(const openvdb::FloatGrid::Ptr);
void computeMeanCurvature(const openvdb::math::Transform, const openvdb::FloatGrid::Ptr, std::map<openvdb::Coord, double>&);
double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>&, std::map<openvdb::Coord, double>&);

openvdb::GridBase::Ptr readFile(const std::string file_name) {
    openvdb::io::File file(file_name);
    file.open();
    openvdb::GridBase::Ptr _grid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
    {
        _grid = file.readGrid(nameIter.gridName());
        // Read in only the grid we are interested in.
        //        if (nameIter.gridName() == "grid_name") {
        //            baseGrid_sphere = file2.readGrid(nameIter.gridName());
        //        } else {
        //            display("Skipping grid", nameIter.gridName());
        //        }
    }
    file.close();
    return _grid;
}

void adjustScale(const openvdb::FloatGrid::Ptr& source_pointer, const openvdb::FloatGrid::Ptr& target_pointer, const float voxel_size = 0.3f) {
    openvdb::FloatGrid::Ptr scaled_grid;
    double  source_area, target_area, area_ratio;
    
    source_area = measureGrid(source_pointer);
    target_area = measureGrid(target_pointer);
    area_ratio =  source_area / target_area;
    
    if(!(area_ratio >= 0.95 && area_ratio <= 1.05)) {           //greater than +/- 5%
        //sclaing up
        if(source_area < target_area) {
            source_pointer->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
            source_pointer->transform().postScale(target_area / source_area);
            source_pointer->pruneGrid();
            writeToFile("scaled_souce.vdb", source_pointer);
        }
        else {
            target_pointer->setTransform(openvdb::math::Transform::createLinearTransform(voxel_size));
            target_pointer->transform().postScale(source_area / target_area);
            target_pointer->pruneGrid();
            writeToFile("scaled_target.vdb", target_pointer);
        }
        source_area = measureGrid(source_pointer);
        target_area = measureGrid(target_pointer);
    }
    
}

int main()
{
    openvdb::initialize();
    
    static const openvdb::Real LEVEL_SET_HALF_WIDTH = 3;
//    openvdb::FloatGrid::Ptr source_grid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
//    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    
    openvdb::FloatGrid::Ptr source_grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(5.0f, openvdb::Vec3f(0.0f), 0.3f);
    openvdb::FloatGrid::Ptr target_grid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid>(10.0f, openvdb::Vec3f(0.0f), 0.3f, float(LEVEL_SET_HALF_WIDTH));
  
//    openvdb::FloatGrid::Ptr source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("cube.vdb"));
//    openvdb::FloatGrid::Ptr target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(readFile("sphere.vdb"));
    
    adjustScale(source_grid, target_grid);
    
    float outside = source_grid->background();
    outside = target_grid->background();
    
    const openvdb::math::Transform &source_trans = source_grid->transform();
    
    openvdb::util::NullInterrupter* interrupt = nullptr;
    openvdb::tools::LevelSetMorphing <openvdb::FloatGrid, openvdb::util::NullInterrupter> ls_morph(*(source_grid), *(target_grid), interrupt);
    
    size_t CFL_count = 10;
    double time = 0.0, time_inc = 1.0, energy_consumed = 0.0, prev_energy = 0.0, total_energy = 0.0, threshold = 0.00001;
    std::string file_name = "";
    
    
    std::map<openvdb::Coord, double> source_coord_meancurv, target_coord_meancurv;
    
    computeMeanCurvature(source_trans, source_grid, source_coord_meancurv);
    
    
    while(true) {
        if(time != 0.0) {
            source_coord_meancurv = target_coord_meancurv;
            
            std::cout << "Energy diff: " << prev_energy - energy_consumed << std::endl;
            
            if(openvdb::math::Abs(energy_consumed - prev_energy) < threshold)
                break;
            
            prev_energy = energy_consumed;
            std::cout << std::endl;
        }
        
        CFL_count = ls_morph.advect(time, time + time_inc);
        
        //source_grid has been updated after advection
        const openvdb::math::Transform &source_trans_updt = source_grid->transform();
        computeMeanCurvature(source_trans_updt, source_grid, target_coord_meancurv);
        
        energy_consumed = computeMeanSumOfCurvatureDifferences(source_coord_meancurv, target_coord_meancurv);
        total_energy += energy_consumed;
        
        file_name = "advect_" + std::to_string((int)(time/time_inc)) + ".vdb";
        writeToFile(file_name, source_grid);
        display("CFL iterations", CFL_count);
        display("File created", file_name);
        display("Energy", energy_consumed);
        display("Total", total_energy);
        
        time += time_inc;
    }
    display("Energy used", energy_consumed);
    return 0;
}

template<class T1, class T2>
void display(const T1 tag, const T2 msg) {
    tag != "" ? std::cout << tag << " - " << msg << std::endl : std::cout << msg << std::endl;
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



void computeMeanCurvature(const openvdb::math::Transform grid_transform, const openvdb::FloatGrid::Ptr grid_pointer, std::map<openvdb::Coord, double>& coord_meancurv) {
    double alpha, beta;
    int zero = 0, pos = 0, neg = 0;
    openvdb::math::MeanCurvature<openvdb::math::MapBase, openvdb::math::DDScheme::CD_SECOND, openvdb::math::DScheme::CD_2ND> mc_obj;
    
    for (openvdb::FloatGrid::ValueOnIter iter = grid_pointer->beginValueOn(); iter; ++iter) {
        
        if(iter.getValue() == 0) {
            zero++;
        }
        else if(iter.getValue() > 0) {
            pos++;;
        }
        else if(iter.getValue() < 0) {
            neg++;
        }
        
        mc_obj.compute(*grid_transform.baseMap(), grid_pointer->getAccessor(), iter.getCoord(), alpha, beta);
        if(beta != 0.0) {
            coord_meancurv[iter.getCoord()] = openvdb::math::Abs(alpha / beta);
        }
        else
            std::cout << "Found zero denominator (beta val)";
    }
    std::cout << zero << " " << pos << " " << neg << std::endl;
}

double computeMeanSumOfCurvatureDifferences(std::map<openvdb::Coord, double>& source_coord_meancurv, std::map<openvdb::Coord, double>& target_coord_meancurv) {
    try {
        double sum_of_differences = 0.0;
        
        for (std::map<openvdb::Coord, double>::iterator it = source_coord_meancurv.begin(); it != source_coord_meancurv.end(); ++it) {
            //if check is not performed, target coords are increased to accomodate default value
            sum_of_differences += (target_coord_meancurv.count(it->first) > 0 ? target_coord_meancurv[it->first] : 0) - source_coord_meancurv[it->first];
        }
        return sum_of_differences / source_coord_meancurv.size();
    }
    catch(...) {
        std::cout << "Error" << std::endl;
        return 0;
    }
}








void old_code() {
    //    openvdb::io::File file1("cube.vdb");
    //    file1.open();
    //    openvdb::GridBase::Ptr baseGrid_cube;
    //    for (openvdb::io::File::NameIterator nameIter = file1.beginName(); nameIter != file1.endName(); ++nameIter)
    //    {
    //        // Read in only the grid we are interested in.
    //        if (nameIter.gridName() == "ls_cube") {
    //            baseGrid_cube = file1.readGrid(nameIter.gridName());
    //        } else {
    //            display("Skipping grid", nameIter.gridName());
    //        }
    //    }
    //    file1.close();
    //    openvdb::io::File file2("sphere.vdb");
    //    file2.open();
    //    openvdb::GridBase::Ptr baseGrid_sphere;
    //    for (openvdb::io::File::NameIterator nameIter = file2.beginName(); nameIter != file2.endName(); ++nameIter)
    //    {
    //        // Read in only the grid we are interested in.
    //        if (nameIter.gridName() == "ls_sphere") {
    //            baseGrid_sphere = file2.readGrid(nameIter.gridName());
    //        } else {
    //            display("Skipping grid", nameIter.gridName());
    //        }
    //    }
    //    file2.close();
    
    //    openvdb::FloatGrid::Ptr cube_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid_cube);
    //    openvdb::FloatGrid::Ptr sphere_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid_sphere);
    //
    //    typedef openvdb::FloatGrid::ValueType ValueT;
    //
    //    const ValueT outside = sphere_grid->background();
    //    const ValueT inside = -outside;
    //
    //    int padding = int(openvdb::math::RoundUp(openvdb::math::Abs(outside)));
    //    // The bounding box of the narrow band is 2*dim voxels on a side.
    //
    //
    //    float radius = 2.5;
    //
    //    std::cout << radius;
    //    int dim = int(radius + padding);
    //    openvdb::FloatGrid::Accessor accessor = sphere_grid->getAccessor();
    //
    //    openvdb::Coord ijk;
    //    openvdb::Vec3f c(0.0, 0.0, 0.0);
    //
    //    int &i = ijk[0], &j = ijk[1], &k = ijk[2];
    //
    //    for (i = c[0] - dim; i < c[0] + dim; ++i) {
    //        const float x2 = openvdb::math::Pow2(i - c[0]);
    //        for (j = c[1] - dim; j < c[1] + dim; ++j) {
    //            const float x2y2 = openvdb::math::Pow2(j - c[1]) + x2;
    //            for (k = c[2] - dim; k < c[2] + dim; ++k) {
    //                const float dist = openvdb::math::Sqrt(x2y2 + openvdb::math::Pow2(k - c[2])) - radius;
    //                ValueT val = ValueT(dist);
    //                if (val < inside || outside < val) continue;
    //                accessor.setValue(ijk, val);
    //            }
    //        }
    //    }
    //    
    //    openvdb::tools::signedFloodFill(sphere_grid->tree());
    //    
    //    writeToFile("narrow_band_sphere.vdb", "narrow_band_sphere", sphere_grid);
}