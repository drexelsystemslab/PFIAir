// Kinect_mesh_grabber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <pcl\PCLPointCloud2.h>
#include <pcl\io\openni2_grabber.h>
#include <pcl\io\pcd_io.h>
#include <pcl\visualization\cloud_viewer.h>
#include <pcl\console\parse.h>
#include <pcl\exceptions.h>
#include <pcl\kdtree\kdtree_flann.h>
#include <pcl\features\normal_3d.h>
#include <pcl\surface\gp3.h>
#include <pcl\point_types.h>
#include <iostream>

using namespace std;
using namespace pcl;
PointCloud<PointXYZRGBA>::Ptr cloudptr(new PointCloud<PointXYZRGBA>); // A cloud that will store color info.
PointCloud<PointXYZ>::Ptr fallbackCloud(new PointCloud<PointXYZ>);    // A fallback cloud with just depth data.
boost::shared_ptr<visualization::CloudViewer> viewer;                 // Point cloud viewer object.
Grabber* openniGrabber;                                               // OpenNI grabber that takes data from the device.
unsigned int filesSaved = 0;                                          // For the numbering of the clouds saved to disk.
bool saveCloud(false), noColor(false);                                // Program control.

void
printUsage(const char* programName)
{
	cout << "Usage: " << programName << " [options]"
		<< endl
		<< endl
		<< "Options:\n"
		<< endl
		<< "\t<none>     start capturing from an OpenNI device.\n"
		<< "\t-v FILE    visualize the given .pcd file.\n"
		<< "\t-h         shows this help.\n";
}

void cloudToMesh(const PointCloud<PointXYZRGBA>::ConstPtr& cloud) {
	cout << "test";
	// Normal estimation*
	pcl::NormalEstimation<PointXYZRGBA, Normal> n;
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGBA>);
	tree->setInputCloud(cloud);
	n.setInputCloud(cloud);
	n.setSearchMethod(tree);
	n.setKSearch(20);
	n.compute(*normals);
	//* normals should not contain the point normals + surface curvatures

	// Concatenate the XYZ and normal fields*
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
	pcl::concatenateFields(*cloud, *normals, *cloud_with_normals);
	//* cloud_with_normals = cloud + normals

	// Create search tree*
	/*pcl::search::KdTree<pcl::PointNormal>::Ptr tree2(new pcl::search::KdTree<pcl::PointNormal>);
	tree2->setInputCloud(cloud_with_normals);*/

	//// Initialize objects
	//pcl::GreedyProjectionTriangulation<pcl::PointNormal> gp3;
	//pcl::PolygonMesh triangles;

	//// Set the maximum distance between connected points (maximum edge length)
	//gp3.setSearchRadius(0.025);

	//// Set typical values for the parameters
	//gp3.setMu(2.5);
	//gp3.setMaximumNearestNeighbors(100);
	//gp3.setMaximumSurfaceAngle(M_PI / 4); // 45 degrees
	//gp3.setMinimumAngle(M_PI / 18); // 10 degrees
	//gp3.setMaximumAngle(2 * M_PI / 3); // 120 degrees
	//gp3.setNormalConsistency(false);

	//// Get result
	//gp3.setInputCloud(cloud_with_normals);
	//gp3.setSearchMethod(tree2);
	//gp3.reconstruct(triangles);

	//// Additional vertex information
	//std::vector<int> parts = gp3.getPartIDs();
	//std::vector<int> states = gp3.getPointStates();
}

// This function is called every time the device has new data.
void
grabberCallback(const PointCloud<PointXYZRGBA>::ConstPtr& cloud)
{
	if (!viewer->wasStopped())
		viewer->showCloud(cloud);

	if (saveCloud)
	{
		cloudToMesh(cloud);
		/*stringstream stream;
		stream << "inputCloud" << filesSaved << ".pcd";
		string filename = stream.str();

		if (io::savePCDFile(filename, *cloud, true) == 0)
		{
			filesSaved++;
			cout << "Saved " << filename << "." << endl;
		}
		else PCL_ERROR("Problem saving %s.\n", filename.c_str());
*/
		saveCloud = false;
	}
}

// For detecting when SPACE is pressed.
void
keyboardEventOccurred(const visualization::KeyboardEvent& event,
	void* nothing)
{
	if (event.getKeySym() == "space" && event.keyDown())
		saveCloud = true;
}

// Creates, initializes and returns a new viewer.
boost::shared_ptr<visualization::CloudViewer>
createViewer()
{
	boost::shared_ptr<visualization::CloudViewer> v
	(new visualization::CloudViewer("OpenNI viewer"));
	v->registerKeyboardCallback(keyboardEventOccurred);

	return (v);
}

int
main(int argc, char** argv)
{
	if (console::find_argument(argc, argv, "-h") >= 0)
	{
		printUsage(argv[0]);
		return -1;
	}

	bool justVisualize(false);
	string filename;
	if (console::find_argument(argc, argv, "-v") >= 0)
	{
		if (argc != 3)
		{
			printUsage(argv[0]);
			return -1;
		}

		filename = argv[2];
		justVisualize = true;
	}
	else if (argc != 1)
	{
		printUsage(argv[0]);
		return -1;
	}

	try {
		openniGrabber = new io::OpenNI2Grabber();
		if (openniGrabber == 0)
			return -1;
		boost::function<void(const PointCloud<PointXYZRGBA>::ConstPtr&)> f =
			boost::bind(&grabberCallback, _1);
		openniGrabber->registerCallback(f);
	}
	catch (io::IOException& e) {
		cout << "issue with opening camera";
		exit(1);
	}

	viewer = createViewer();

	if (justVisualize)
	{
		if (noColor)
			viewer->showCloud(fallbackCloud);
		else viewer->showCloud(cloudptr);
	}
	else openniGrabber->start();

	// Main loop.
	while (!viewer->wasStopped())
		boost::this_thread::sleep(boost::posix_time::seconds(1));

	if (!justVisualize)
		openniGrabber->stop();
}
