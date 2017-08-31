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

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/betweenness_centrality.hpp>

namespace PFIAir {
    using namespace openvdb;
    using namespace std;
    
    Container::Container() {
        initialize();
    }
    
    void Container::setScale(Vec3d scale) {
        this -> _scale.preScale(scale);
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
    
    void Container::computeMeshCenter() {
        using namespace boost;
        
        typedef std::pair<Vec3s, Vec3s> Edge;
        std::vector<Edge> edges;
        
        for (int i = 0; i < _indicesTri.size(); i++) {
            Vec3s point1 = _points[_indicesTri[i].x()];
            Vec3s point2 = _points[_indicesTri[i].y()];
            Vec3s point3 = _points[_indicesTri[i].z()];
            
            Edge edge1 = Edge(point1, point2);
            Edge edge2 = Edge(point1, point3);
            Edge edge3 = Edge(point2, point3);
            
            edges.push_back(edge1);
            edges.push_back(edge2);
            edges.push_back(edge3);
        }
        
        for (int i = 0; i < _indicesQuad.size(); i++) {
            Vec3s point1 = _points[_indicesQuad[i].x()];
            Vec3s point2 = _points[_indicesQuad[i].y()];
            Vec3s point3 = _points[_indicesQuad[i].z()];
            Vec3s point4 = _points[_indicesQuad[i].w()];
            
            Edge edge1 = Edge(point1, point2);
            Edge edge2 = Edge(point2, point3);
            Edge edge3 = Edge(point3, point4);
            Edge edge4 = Edge(point4, point1);
            
            edges.push_back(edge1);
            edges.push_back(edge2);
            edges.push_back(edge3);
            edges.push_back(edge4);
        }
        
        typedef adjacency_list<Vec3s, Vec3s, bidirectionalS,
        property<Vec3s, Vec3s>> Graph;
        
        Graph g(edges.begin(), edges.end(), edges.size());
        
        shared_array_property_map<double, property_map<Graph, vertex_index_t>::const_type>
        centrality_map(num_vertices(g), get(boost::vertex_index, g));
        
        brandes_betweenness_centrality(g, centrality_map);
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
    
    float Container::computeAverageEdgeLength() {
        vector<float> tri_avg = vector<float>();
        vector<float> quad_avg = vector<float>();

        // average length for triangles
        for (int i = 0; i < _indicesTri.size(); i++) {
            Vec3s point1 = _points[_indicesTri[i].x()];
            Vec3s point2 = _points[_indicesTri[i].y()];
            Vec3s point3 = _points[_indicesTri[i].z()];

            Vec3s edge1 = point1 - point2;
            Vec3s edge2 = point1 - point3;
            Vec3s edge3 = point2 - point3;
            
            float avg = (edge1.length() + edge2.length() + edge3.length()) / 3.0;
            tri_avg.push_back(avg);
        }
        
        // average length for quads
        for (int i = 0; i < _indicesQuad.size(); i++) {
            Vec3s point1 = _points[_indicesQuad[i].x()];
            Vec3s point2 = _points[_indicesQuad[i].y()];
            Vec3s point3 = _points[_indicesQuad[i].z()];
            Vec3s point4 = _points[_indicesQuad[i].w()];
            
            Vec3s edge1 = point1 - point2;
            Vec3s edge2 = point2 - point3;
            Vec3s edge3 = point3 - point4;
            Vec3s edge4 = point4 - point1;

            
            float avg = (edge1.length() + edge2.length() + edge3.length() + edge4.length()) / 4.0;
            quad_avg.push_back(avg);
        }
        
        // compute overall average
        float tri_sum = 0, quad_sum = 0;
        for (int i = 0; i < tri_avg.size(); i++) {
            tri_sum += tri_avg[i];
        }
        
        for (int i = 0; i < quad_avg.size(); i++) {
            quad_sum += quad_avg[i];
        }
        
        float weighted = (tri_sum + quad_sum) / (tri_avg.size() + quad_avg.size());
        
        return weighted;
    }
}
