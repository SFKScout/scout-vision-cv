#include "pch.h"
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/point_types.h>
#include <pcl/conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>

using namespace std;
using namespace pcl;

int main()
{
    PCLPointCloud2::Ptr cloudIn(new PCLPointCloud2());
    PCLPointCloud2::Ptr cloudOut(new PCLPointCloud2());
    PCLPointCloud2::Ptr cloudOutPlane(new PCLPointCloud2());
    PLYReader reader;
    
    reader.read("D:\\Projects\\Scout\\Pipeline\\tango.ply", *cloudIn);
    
    cerr << "PointCloud before filtering: " << cloudIn->width * cloudIn->height
        << " data points (" << getFieldsList(*cloudIn) << ")." << endl;

    VoxelGrid<pcl::PCLPointCloud2> filter;
    filter.setInputCloud(cloudIn);
    filter.setLeafSize(1.0f, 1.0f, 1.0f);
    filter.filter(*cloudOut);
    
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
    pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
    // Create the segmentation object
    pcl::SACSegmentation<pcl::PointXYZ> seg;
    // Optional
    seg.setOptimizeCoefficients(true);
    // Mandatory
    seg.setModelType(pcl::SACMODEL_PLANE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setDistanceThreshold(2.0f);
    
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloudDownsampled(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromPCLPointCloud2(*cloudOut, *cloudDownsampled);

    seg.setInputCloud(cloudDownsampled);
    seg.segment(*inliers, *coefficients);

    cerr << "PointCloud after filtering: " << cloudOut->width * cloudOut->height
        << " data points (" << getFieldsList(*cloudOut) << ")." << endl;

    if (inliers->indices.size() == 0)
    {
        PCL_ERROR("Could not estimate a planar model for the given dataset.\n");
        return (-1);
    }
    
    std::cerr << "Model coefficients: " << coefficients->values[0] << " "
        << coefficients->values[1] << " "
        << coefficients->values[2] << " "
        << coefficients->values[3] << std::endl;
    
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloudPlane(new pcl::PointCloud<pcl::PointXYZ>);

    std::cerr << "Model inliers: " << inliers->indices.size() << std::endl;
    int planeIdx = 0;
    for (const auto& idx : inliers->indices)
    {
        /*std::cerr << idx << "    " << cloudDownsampled->points[idx].x << " "
            << cloudDownsampled->points[idx].y << " "
            << cloudDownsampled->points[idx].z << std::endl;
*/
        cloudPlane->push_back(cloudDownsampled->points[idx]);
    }

    printf("plane data size: %zd\n", cloudPlane->size());
    pcl::toPCLPointCloud2(*cloudPlane, *cloudOutPlane);

    PLYWriter writer;
    writer.write("D:\\Projects\\Scout\\Pipeline\\tango_downsampled.ply", *cloudOutPlane,
        Eigen::Vector4f::Zero(), Eigen::Quaternionf::Identity(), false, true);
        
    return (0);
}
