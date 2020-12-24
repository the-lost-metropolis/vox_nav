// Copyright (c) 2020 Fetullah Atas, Norwegian University of Life Sciences
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * Parts of code has been taken from
 *      Edo Jelavic
 *      Institute: ETH Zurich, Robotic Systems Lab
 */

#include <memory>
#include <string>
#include <pcl/common/common.h>
#include <eigen3/Eigen/Core>


#include <pcl/common/pca.h>
#include <pcl/common/transforms.h>
#include <pcl/conversions.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_types.h>
#include <pcl/segmentation/extract_clusters.h>

namespace botanbot_utilities
{

/**
 * @brief
 *
 * @param inputCloud
 * @return Eigen::Vector3d
 */
Eigen::Vector3d calculateMeanOfPointPositions(pcl::PointCloud<pcl::PointXYZ>::ConstPtr inputCloud);

/**
 * @brief
 *
 * @param inputCloud
 * @param transformMatrix
 * @return pcl::PointCloud<pcl::PointXYZ>::Ptr
 */
pcl::PointCloud<pcl::PointXYZ>::Ptr transformCloud(
  pcl::PointCloud<pcl::PointXYZ>::ConstPtr inputCloud,
  const Eigen::Affine3f & transformMatrix);

/**
 * @brief
 *
 * @param filename
 * @return pcl::PointCloud<pcl::PointXYZ>::Ptr
 */
pcl::PointCloud<pcl::PointXYZ>::Ptr loadPointcloudFromPcd(const std::string & filename);

/*!
 * Finds clusters in the input cloud and returns vector point clouds.
 * Each pointcloud in the vector is a cluster in the input cloud.
 * There can be more than one cluster.
 * @param[in] pointer to the pcl point cloud
 * @return vector of point clouds. Vector will be empty if no clusters are found.
 */
std::vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> extractClusterCloudsFromPointcloud(
  pcl::PointCloud<pcl::PointXYZ>::Ptr inputCloud);
}  // namespace botanbot_utilities