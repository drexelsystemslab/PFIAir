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
#include <tuple>
#include <algorithm>
#include <iterator>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/eccentricity.hpp>

namespace PFIAir {
    using namespace openvdb;
    using namespace std;
    
    Container::Container() {
        initialize();
    }
    
    void Container::setScale(Vec3d scale) {
        this -> _scale.preScale(scale);
    }
    
    /**
     * Import index and face defined model files like obj or smf file.
     *
     * TODO: Why do we have both this and loadOBJ in UpdtMeshOperations?
     * this seems redundant.
     */
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
                        // TODO: The new open meshes are different somehow
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
    
    void Container::OLDcomputeMeshCenter() {
        using namespace boost;

        typedef std::pair<int, int> Edge;
        std::vector<Edge> edges;

        for (int i = 0; i < _indicesTri.size(); i++) {
            int point1 = _indicesTri[i].x();
            int point2 = _indicesTri[i].y();
            int point3 = _indicesTri[i].z();

            Edge edge1 = Edge(point1, point2);
            Edge edge2 = Edge(point1, point3);
            Edge edge3 = Edge(point2, point3);

            edges.push_back(edge1);
            edges.push_back(edge2);
            edges.push_back(edge3);
        }

        for (int i = 0; i < _indicesQuad.size(); i++) {
            int point1 = _indicesQuad[i].x();
            int point2 = _indicesQuad[i].y();
            int point3 = _indicesQuad[i].z();
            int point4 = _indicesQuad[i].w();

            Edge edge1 = Edge(point1, point2);
            Edge edge2 = Edge(point2, point3);
            Edge edge3 = Edge(point3, point4);
            Edge edge4 = Edge(point4, point1);

            edges.push_back(edge1);
            edges.push_back(edge2);
            edges.push_back(edge3);
            edges.push_back(edge4);
        }

        typedef adjacency_list<vecS, vecS, undirectedS> Graph;

        Graph g(edges.begin(), edges.end(), edges.size());


                shared_array_property_map<double, property_map<Graph, vertex_index_t>::const_type>
                centrality_map(num_vertices(g), get(boost::vertex_index, g));
        
                brandes_betweenness_centrality(g, centrality_map);


       // cout << num_vertices(g) << endl;

                double max = 0;
                int maxIndex = 0;
                for (int i = 0; i < num_vertices(g); i++) {
                    if (centrality_map[i] > max) {
                        max = centrality_map[i];
                        maxIndex = i;
                    }
                }
        
    }
    
    typedef std::vector<std::pair<double,Vec3s>> Result;
    
    bool comp (std::pair<double,Vec3s> left, std::pair<double,Vec3s> right) {
        return left.first > right.first;
    }

    // Insert a value into the result in sorted order
    // This takes O(n log n) time
    void insert( Result &cont, std::pair<double,Vec3s> value ) {
        // O(log n)
        Result::iterator it = std::lower_bound( cont.begin(), cont.end(), value, comp);
        // O(n)
        cont.insert( it, value );
    }
    
    /**
     * Check if the edge (a, b) has NO duplicates in the graph
     *
     * This implementation is O(E).
     *
     * TODO: Maybe use a hash table for edges instead?
     * TODO: This should be named better. this returns true when there
     * are NO duplicates.
     */
    bool edgeDup(std::vector<std::pair<int, int>> edges, int a, int b) {
        // TODO: Just compare std::pair<int, int> objects?
        for (int i = 0; i < edges.size(); i++) {
            // If (a, b) or (b, a) is in the edge list, return false
            if ((edges[i].first == a && edges[i].second == b) || (edges[i].first == b && edges[i].second == a)) {
                return false;
            }
        }
        return true;
    }
    
    double Container::getEuclideanFromIndices(int p1, int p2){
        using namespace openvdb;
        Vec3s point1 = _points[p1];
        Vec3s point2 = _points[p2];
        
        Vec3s diff = point1.sub(point2, point1);
        
        return (double)diff.length();
    }
    
    /**
     * Overall time complexity: 
     * 
     * construct graph + compute distances + compute eccentricity + sort results
     * O(num_faces * log(V + E)) + O(V^3) + O(V^2) + O(V^2 log V)
     *
     * The biggest time hog is the Floyd-Warshall algorithm for computing
     * distances. For all but the smallest meshes, this is impractically
     * slow (well over 20 min for one model!).
     * 
     * Some of the smaller terms could be handled more efficiently
     * (in particular, that last O(V^2 log V) sort routine should
     * be O(V log V) instead, or even better: just find the minimum
     * in O(V) time. Also, I think the graph might be better off with
     * a hash table structure so insertions are faster.
     * 
     * Space complexity: not sure, my guess is Omega(V^2)
     * Be careful with large meshes (on the order of 10k+ vertices), this
     * may chew up 8GB+ of RAM. 
     */
    void Container::computeMeshCenter() {
        using namespace boost;

        // Edges are weighted with doubles
        // TODO: what do the weights represent?
        typedef boost::property<
            boost::edge_weight_t, double> EdgeWeightProperty;
//        typedef adjacency_list<vecS, vecS, undirectedS,boost::no_property, EdgeWeightProperty
//        > Graph;

        // Graphs are represented as an undirected adjacency list, i.e. 
        // jagged 2D list of vertex then adjacent vertices.
        typedef adjacency_list<
            vecS, // vecS means use std::vector for edges 
            vecS, // vecS means use std::vector for vertices
            undirectedS, 
            boost::no_property, 
            EdgeWeightProperty> Graph;

        // Vertex and edge types within the adjacency list
        typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
        typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

        // What are these tables for?

        // Map of vertex indices to vertices in the graphs
        // note that this is a tree-based data structure so it has
        // O(log V) complexity
        std::map<int, vertex_t> m;

        // List of vertices
        std::vector<int> ver;
        
        // Graph representation of this model
        Graph g;

        // Another list of edges? why?
        std::vector<std::pair<int, int>> edges;
        
        // Add the quads to the graph
        // Time complexity: num_quads * (O(log V) + O(1) + O(E))
        //      = O(num_quads * (log V + E))
        for (int i = 0; i < _indicesQuad.size(); i++) {
            // Get the four vertices of the quad
            int point1 = _indicesQuad[i].x();
            int point2 = _indicesQuad[i].y();
            int point3 = _indicesQuad[i].z();
            int point4 = _indicesQuad[i].w();
             
            // Add vertices to the graph
            // if they are not already there
            // O(log V) overall due to map insertions and map search.
            if (m.find(point1) == m.end()) {
                // Add a vertext to the graph and store
                // the vertex in the map.
                // add_vertex() takes O(1) time for a std::vector
                // the map insertion, however takes O(log V) time
                m[point1] = (vertex_t)add_vertex(g);
                // Store the vertex list here
                ver.push_back(point1);
            }
            if (m.find(point2) == m.end()) {
                m[point2] = (vertex_t)add_vertex(g);
                ver.push_back(point2);
            }
            if (m.find(point3) == m.end()) {
                m[point3] = (vertex_t)add_vertex(g);
                ver.push_back(point3);
            }
            
            if (m.find(point4) == m.end()) {
                m[point4] = (vertex_t)add_vertex(g);
                ver.push_back(point4);
            }
            
            // Now that the vertices are updated,
            // get the updated vertex complexity
            // O(1) time complexity overall
            vertex_t d1 = m.find(point1)->second;
            vertex_t d2 = m.find(point2)->second;
            vertex_t d3 = m.find(point3)->second;
            vertex_t d4 = m.find(point4)->second;
           

            // if the edges of this quad are new, add the edge to the graph
            // and 
            // EdgeDup is O(E)
            // Everything else is O(1)
            // Overall, we have O(E) time complexity
            // TODO: consider using std::set for edges for uniqueness
            // in O(log(E / V)) complexity instead?
            if (edgeDup(edges, point1, point2)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point1, point2);
                add_edge(d1, d2, e, g);
                edges.push_back(std::pair<int, int>(point1, point2));
            }
            if (edgeDup(edges, point2, point3)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point2, point3);
                add_edge(d2, d3, e, g);
                edges.push_back(std::pair<int, int>(point2, point3));
            }
            if (edgeDup(edges, point3, point4)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point3, point4);
                add_edge(d3, d4, e, g);
                edges.push_back(std::pair<int, int>(point3, point4));
            }
            if (edgeDup(edges, point4, point1)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point4, point1);
                add_edge(d4, d1, e, g);
                edges.push_back(std::pair<int, int>(point4, point1));
            }
        }

        std::cout << "Done with quad indices " << std::endl;
        
        // Do the same for the triangles
        // O(num_triangles * (log V + E)
        for (int i = 0; i < _indicesTri.size(); i++) {
            int point1 = _indicesTri[i].x();
            int point2 = _indicesTri[i].y();
            int point3 = _indicesTri[i].z();
            
            //O(log V)
            if (m.find(point1) == m.end()) {
                m[point1] = (vertex_t)add_vertex(g);
                ver.push_back(point1);
            }
            if (m.find(point2) == m.end()) {
                m[point2] = (vertex_t)add_vertex(g);
                ver.push_back(point2);
            }
            if (m.find(point3) == m.end()) {
                m[point3] = (vertex_t)add_vertex(g);
                ver.push_back(point3);
            }

            vertex_t d1 = m.find(point1)->second;
            vertex_t d2 = m.find(point2)->second;
            vertex_t d3 = m.find(point3)->second;

            //O(E)
            //add_edge(d1,d2,EdgeWeightProperty(2),g);
            if (edgeDup(edges, point1, point2)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point1, point2);
                add_edge(d1,d2,e,g);
                edges.push_back(std::pair<int, int>(point1, point2));
            }
            if (edgeDup(edges, point1, point3)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point1, point3);
                add_edge(d1,d3,e,g);
                edges.push_back(std::pair<int, int>(point1, point3));
            }
            if (edgeDup(edges, point2, point3)) {
                EdgeWeightProperty e = getEuclideanFromIndices(point2, point3);
                add_edge(d2,d3,e,g);
                edges.push_back(std::pair<int, int>(point2, point3));
            }
        }

        std::cout << "after triangles" << std::endl;
        
//        shared_array_property_map<double, property_map<Graph, vertex_index_t>::const_type>
//        centrality_map(num_vertices(g), get(boost::vertex_index, g));
//        
        // O(1) time
        // Get a property map for the edge weights for passing into
        // graph algorithms.
        property_map<Graph, edge_weight_t>::type w = get(edge_weight, g);

        std::cout << "after get" << std::endl;
//
//        brandes_betweenness_centrality(g, boost::centrality_map(centrality_map).weight_map(w));
        
        // Get the 
        typedef graph_traits<Graph>::edge_descriptor Edge;
        typedef exterior_vertex_property<Graph, double> DistanceProperty;
        typedef DistanceProperty::matrix_type DistanceMatrix;
        typedef DistanceProperty::matrix_map_type DistanceMatrixMap;

        // This property map is for the eccentricity calculation
        typedef constant_property_map<Edge, double> ECCWeightMap;

        // yikes, the segmented models are huge. 43.7k vertices
        // compared with the old models which were an order of magnitude
        // smaller
        std::cout << "num_vertices: " << num_vertices(g) << std::endl;

        // is this a dense matrix of size V x V? if so, that would
        // explain why this is using up all my memory.
        // 43.7k items * 43.7k = 1.9G items. If there were V x V elements
        // of 4 bytes each, that is already 7.6 GB of memory! 
        DistanceMatrix distances(num_vertices(g));
        std::cout << sizeof(distances) << std::endl;

        std::cout << "constructed distance matrix" << std::endl;

        DistanceMatrixMap dm(distances, g);

        std::cout << "constructed Distance Matrix Map" << std::endl; 
        // Weight map for the eccentricity
        ECCWeightMap wm(1);

        std::cout << "before warshall" << std::endl;

        // Floyd Warshall is expensive, at O(V^3), yikes!
        // TODO: Can we use another algorithm?
        //floyd_warshall_all_pairs_shortest_paths(g, dm, w);
        floyd_warshall_all_pairs_shortest_paths(g, dm, weight_map(w));

        std::cout << "after floyd_warshall" << std::endl;
        
        // Compute the eccentricity information given the 
        typedef boost::exterior_vertex_property<Graph, double> EccentricityProperty;
        typedef EccentricityProperty::container_type EccentricityContainer;
        typedef EccentricityProperty::map_type EccentricityMap;
        
        int r, d;
        EccentricityContainer eccs(num_vertices(g));
        EccentricityMap em(eccs, g);

        // This is another O(V^2) operation.
        boost::tie(r, d) = all_eccentricities(g, dm, em);
        
        // Construct a sorted list of vertices by eccentricity
        // TODO: It would be better to just dump everything into
        // a vector and sort at the end in O(V log V) time.
        // NOTE: This is no longer used.
        Result sorted_result;
        map<int, vertex_t>::iterator it;
        int c = 0;
        // This is not done very efficiently.
        // this is V * O(V log V) = O(V^2 log V))
        for ( it = m.begin(); it != m.end(); it++ ) { 
            // insert takes O(V log V) time
            insert(
                sorted_result, 
                std::pair<double, Vec3s>(em[c], _points[ver.at(c)])); 
            c++;
        }
        
        return;
        

//        Result sorted_result;
//        map<int, vertex_t>::iterator it;
//        int c = 0;
//        for ( it = m.begin(); it != m.end(); it++ ) {
//
//            insert(sorted_result, std::pair<double, Vec3s>(centrality_map[c], _points[ver.at(c)]));
//
//            c++;
//        }
//        
        // This section was from the mac_tools branch
//        for (int i = 0; i < sorted_result.size(); i++) {
//            cout << sorted_result[i].first << " " << sorted_result[i].second << endl;
//        }
//        
//        cout << "\nvertices: " << num_vertices(g) << endl;
//        cout << "edges: " << num_edges(g) << endl;
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
