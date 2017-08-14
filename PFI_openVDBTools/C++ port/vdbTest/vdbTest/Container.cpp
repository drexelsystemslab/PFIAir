//
//  Container.cpp
//  vdbTest
//
//  Created by Hanjie Liu on 7/28/17.
//  Copyright © 2017 Hanjie Liu. All rights reserved.
//
// code partially from http://www.cplusplus.com/doc/tutorial/files/
// and https://stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string

#include "Container.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <openvdb/openvdb.h>
#include <openvdb/points/PointConversion.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/MeshToVolume.h>

namespace PFIAir {
    using namespace openvdb;
    using namespace std;
    
    float DISTANCE_OFFSET = 0.01;
    
    Container::Container(Vec3d scale) {
        this -> _scale.preScale(scale);
        
        initialize();
    }
    
    /// import index and face defined model files like obj or smf file
    void Container::loadMeshModel(const string filename) {
        this -> _filename = filename;
        
        string line;
        ifstream myfile (filename);
        
        if (myfile.is_open())
        {
            while ( getline (myfile,line) )
            {
                
                istringstream iss(line);
                vector<string> tokens{istream_iterator<string>{iss},
                    istream_iterator<string>{}};
                
                if (tokens.size() > 0) {
                    // vertices
                    if (!tokens[0].compare("v")) {
                        assert(tokens.size() == 4);

                        this -> _points.push_back(Vec3s(stof(tokens[1]),stof(tokens[2]),stof(tokens[3])));
                        continue;
                    }
                    
                    // facets
                    if (!tokens[0].compare("f")) {
                        assert(tokens.size() == 4 || tokens.size() == 5 );
                        
                        if (tokens.size() == 4) {
                            // fix obj indexing issue by -1
                            _indicesTri.push_back(Vec3I(stoi(tokens[1]) - 1,stoi(tokens[2]) - 1,stoi(tokens[3]) - 1));
                        } else {
                            _indicesQuad.push_back(Vec4I(stoi(tokens[1]) - 1,stoi(tokens[2]) - 1,stoi(tokens[3]) - 1,stoi(tokens[4]) - 1));
                        }
                    }
                }
            }
            myfile.close();
        }
        
        else cout << "Unable to open file" << endl;
    }
    
    FloatGrid::Ptr Container::getWaterTightLevelSet() {
        return tools::meshToLevelSet<FloatGrid>(_scale, _points, _indicesTri, _indicesQuad);
    }
    
    FloatGrid::Ptr Container::getWaterTightLevelSetWithBandWidth(float w) {
        return tools::meshToLevelSet<FloatGrid>(_scale, _points, _indicesTri, _indicesQuad, w);
    }
    
    FloatGrid::Ptr Container::getUnsignedDistanceField(float bandwidth) {
        return tools::meshToUnsignedDistanceField<FloatGrid>(_scale, _points, _indicesTri, _indicesQuad, bandwidth);
    }
    
    void Container::exportModel(const string name, FloatGrid::Ptr model) {
        io::File file(name);
        GridPtrVec grids;
        grids.push_back(model);
        file.write(grids);
        file.close();
    }
    
    /*
    bool Container::inflate(const float offset) {
        //Busing namespace openvdb;
        
        
        
        //static const Real LEVEL_SET_HALF_WIDTH = 5.0;
        
        auto transform = math::Transform();
        Vec3d scale_vec = Vec3d(0.1,0.1,0.1);
        transform.preScale(scale_vec);
        //
        //    cout << transform.voxelSize() << endl;
        //
        //    return false;
        
        //

        
        // offset
        //    for (FloatGrid::ValueAllIter iter = levelSet -> beginValueAll() ; iter; ++iter) {
        //        float dist = iter.getValue();
        //        iter.setValue(dist - DISTANCE_OFFSET);
        //        //cout << iter.getValue() << endl;
        //    }
        //    auto box = uDistance_field -> evalActiveVoxelBoundingBox();
        //
        //    auto out = tools::createLevelSetBox<FloatGrid>(box, transform);
        
        //auto levelSet = tools::meshToUnsignedDistanceField<FloatGrid>(transform, points, indicesTri, indicesQuad, 2.0);

        
        //    initialize();
        //    // Create a FloatGrid and populate it with a narrow-band
        //    // signed distance field of a sphere.
        //    FloatGrid::Ptr grid =

        //    // Associate some metadata with the grid.
        //    grid->insertMeta("radius", FloatMetadata(50.0));
        //    // Name the grid "LevelSetSphere".
        //    grid->setName("LevelSetSphere");
        //    // Create a VDB file object.
        //    io::File file("mygrids.vdb");
        //    // Add the grid pointer to a container.
        //    GridPtrVec grids;
        //    grids.push_back(grid);
        //    // Write out the contents of the container.
        //    file.write(grids);
        //    file.close();
        return true;
    }
    */
}
