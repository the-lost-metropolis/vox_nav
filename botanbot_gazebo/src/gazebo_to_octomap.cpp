/*
 * Copyright 2015 Fadri Furrer, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Mina Kamel, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Janosch Nikolic, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Markus Achtelik, ASL, ETH Zurich, Switzerland
 * Copyright 2016 Geoffrey Hunter <gbmhunter@gmail.com>
 * Copyright 2021 Fetullah Atas, Norwegian University of Life Sciences
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <botanbot_gazebo/gazebo_to_octomap.hpp>
#include <octomap_msgs/conversions.h>
#include <gazebo/common/Time.hh>
#include <gazebo/common/CommonTypes.hh>

namespace gazebo
{

  OctomapFromGazeboWorld::~OctomapFromGazeboWorld()
  {
    delete octomap_;
    octomap_ = NULL;
  }

  void OctomapFromGazeboWorld::Load(
    physics::WorldPtr _parent,
    sdf::ElementPtr _sdf)
  {
    // Initialize ROS node
    node_ = gazebo_ros::Node::Get(_sdf);

    if (kPrintOnPluginLoad) {
      gzdbg << __FUNCTION__ << "() called." << std::endl;
    }
    RCLCPP_INFO(
      node_->get_logger(),
      "Contrcting an instance of gazebo_world_to_octomap_rclcpp_node");

    world_ = _parent;

    std::string service_name = "world/build_octomap";
    std::string octomap_pub_topic = "world/octomap";
    getSdfParam<std::string>(
      _sdf, "octomapPubTopic", octomap_pub_topic,
      octomap_pub_topic);
    getSdfParam<std::string>(
      _sdf, "octomapServiceName", service_name,
      service_name);

    gzlog << "Advertising service: " << service_name << std::endl;

    octomap_publisher_ = node_->create_publisher<octomap_msgs::msg::Octomap>(
      octomap_pub_topic.c_str(), rclcpp::SystemDefaultsQoS());

    // LAMBDA SERVICE CALLBACK
    auto service_callback = [this](
      const std::shared_ptr<vox_nav_msgs::srv::GetOctomap::Request> req,
      std::shared_ptr<vox_nav_msgs::srv::GetOctomap::Response> res)
      {
        RCLCPP_INFO(node_->get_logger(), "Recieved an request to convert gazebo world to octomap");
        CreateOctomap(*req);
        if (req->filename != "") {
          if (octomap_) {
            std::string path = req->filename;
            octomap_->writeBinary(path);
            gzlog << std::endl << "Octree saved as " << path << std::endl;
          } else {
            RCLCPP_ERROR(node_->get_logger(), "The octree is NULL. Will not save that.");
          }
        }
        common::Time now = world_->SimTime();
        res->map.header.frame_id = "world";
        res->map.header.stamp = node_->now();

        if (!octomap_msgs::binaryMapToMsg(*octomap_, res->map)) {
          RCLCPP_ERROR(node_->get_logger(), "Error serializing OctoMap");
        }

        if (req->publish_octomap) {
          gzlog << "Publishing Octomap." << std::endl;
          octomap_publisher_->publish(res->map);
        }

        common::SphericalCoordinatesPtr sphericalCoordinates = world_->SphericalCoords();
        ignition::math::Vector3d origin_cartesian(0.0, 0.0, 0.0);
        ignition::math::Vector3d origin_spherical = sphericalCoordinates->
          SphericalFromLocal(origin_cartesian);

        res->origin_latitude = origin_spherical.X();
        res->origin_longitude = origin_spherical.Y();
        res->origin_altitude = origin_spherical.Z();
        return true;
      };
    // Create a service that will use the callback function to handle requests.
    srv_ = node_->create_service<vox_nav_msgs::srv::GetOctomap>(service_name, service_callback);
  }

  void OctomapFromGazeboWorld::FloodFill(
    const ignition::math::Vector3d & seed_point,
    const ignition::math::Vector3d & bounding_box_origin,
    const ignition::math::Vector3d & bounding_box_lengths, const double leaf_size)
  {
    octomap::OcTreeNode * seed =
      octomap_->search(seed_point.X(), seed_point.Y(), seed_point.Z());
    // do nothing if point occupied
    if (seed != NULL && seed->getOccupancy()) {return;}

    std::stack<octomath::Vector3> to_check;
    to_check.push(octomath::Vector3(seed_point.X(), seed_point.Y(), seed_point.Z()));

    while (to_check.size() > 0) {
      octomath::Vector3 p = to_check.top();

      if ((p.x() > bounding_box_origin.X() - bounding_box_lengths.X() / 2) &&
        (p.x() < bounding_box_origin.X() + bounding_box_lengths.X() / 2) &&
        (p.y() > bounding_box_origin.Y() - bounding_box_lengths.Y() / 2) &&
        (p.y() < bounding_box_origin.Y() + bounding_box_lengths.Y() / 2) &&
        (p.z() > bounding_box_origin.Z() - bounding_box_lengths.Y() / 2) &&
        (p.z() < bounding_box_origin.Z() + bounding_box_lengths.Y() / 2) &&
        (!octomap_->search(p)))
      {
        octomap_->setNodeValue(p, 0);
        to_check.pop();
        to_check.push(octomath::Vector3(p.x() + leaf_size, p.y(), p.z()));
        to_check.push(octomath::Vector3(p.x() - leaf_size, p.y(), p.z()));
        to_check.push(octomath::Vector3(p.x(), p.y() + leaf_size, p.z()));
        to_check.push(octomath::Vector3(p.x(), p.y() - leaf_size, p.z()));
        to_check.push(octomath::Vector3(p.x(), p.y(), p.z() + leaf_size));
        to_check.push(octomath::Vector3(p.x(), p.y(), p.z() - leaf_size));

      } else {
        to_check.pop();
      }
    }
  }

  bool OctomapFromGazeboWorld::CheckIfInterest(
    const ignition::math::Vector3d & central_point,
    gazebo::physics::RayShapePtr ray,
    const double leaf_size)
  {
    ignition::math::Vector3d start_point = central_point;
    ignition::math::Vector3d end_point = central_point;

    double dist;
    std::string entity_name;

    start_point.X() += leaf_size / 2;
    end_point.X() -= leaf_size / 2;
    ray->SetPoints(start_point, end_point);
    ray->GetIntersection(dist, entity_name);

    if (dist <= leaf_size) {return true;}

    start_point = central_point;
    end_point = central_point;
    start_point.Y() += leaf_size / 2;
    end_point.Y() -= leaf_size / 2;
    ray->SetPoints(start_point, end_point);
    ray->GetIntersection(dist, entity_name);

    if (dist <= leaf_size) {return true;}

    start_point = central_point;
    end_point = central_point;
    start_point.Z() += leaf_size / 2;
    end_point.Z() -= leaf_size / 2;
    ray->SetPoints(start_point, end_point);
    ray->GetIntersection(dist, entity_name);

    if (dist <= leaf_size) {return true;}

    return false;
  }

  void OctomapFromGazeboWorld::CreateOctomap(
    const vox_nav_msgs::srv::GetOctomap::Request & msg)
  {
    const double epsilon = 0.00001;
    ignition::math::Vector3d bounding_box_origin(msg.bounding_box_origin.x,
      msg.bounding_box_origin.y,
      msg.bounding_box_origin.z);
    // epsilion prevents undefiened behaviour if a point is inserted exactly
    // between two octomap cells
    ignition::math::Vector3d bounding_box_lengths(msg.bounding_box_lengths.x + epsilon,
      msg.bounding_box_lengths.y + epsilon,
      msg.bounding_box_lengths.z + epsilon);
    double leaf_size = msg.leaf_size;
    octomap_ = new octomap::OcTree(leaf_size);
    octomap_->clear();
    octomap_->setProbHit(0.7);
    octomap_->setProbMiss(0.4);
    octomap_->setClampingThresMin(0.12);
    octomap_->setClampingThresMax(0.97);
    octomap_->setOccupancyThres(0.7);

    gazebo::physics::PhysicsEnginePtr engine = world_->Physics();
    engine->InitForThread();
    gazebo::physics::RayShapePtr ray =
      boost::dynamic_pointer_cast<gazebo::physics::RayShape>(
      engine->CreateShape("ray", gazebo::physics::CollisionPtr()));

    std::cout << "Rasterizing world and checking collisions" << std::endl;

    for (double x = leaf_size / 2 + bounding_box_origin.X() - bounding_box_lengths.X() / 2;
      x < bounding_box_origin.X() + bounding_box_lengths.X() / 2;
      x += leaf_size)
    {
      int progress =
        round(
        100 * (x + bounding_box_lengths.X() / 2 - bounding_box_origin.X()) /
        bounding_box_lengths.X());
      std::cout << "\rPlacing model edges into octomap... " << progress <<
        "%                 ";

      for (double y =
        leaf_size / 2 + bounding_box_origin.Y() - bounding_box_lengths.Y() / 2;
        y < bounding_box_origin.Y() + bounding_box_lengths.Y() / 2;
        y += leaf_size)
      {
        for (double z = leaf_size / 2 + bounding_box_origin.Z() -
          bounding_box_lengths.Z() / 2;
          z < bounding_box_origin.Z() + bounding_box_lengths.Z() / 2;
          z += leaf_size)
        {
          ignition::math::Vector3d point(x, y, z);
          if (CheckIfInterest(point, ray, leaf_size)) {
            octomap_->setNodeValue(x, y, z, 1);
          }
        }
      }
    }
    octomap_->prune();
    octomap_->updateInnerOccupancy();

    // flood fill from top and bottom
    std::cout << "\rFlood filling freespace...                                  ";
    FloodFill(
      ignition::math::Vector3d(
        bounding_box_origin.X() + leaf_size / 2,
        bounding_box_origin.Y() + leaf_size / 2,
        bounding_box_origin.Z() + bounding_box_lengths.Z() / 2 -
        leaf_size / 2),
      bounding_box_origin, bounding_box_lengths, leaf_size);
    FloodFill(
      ignition::math::Vector3d(
        bounding_box_origin.X() + leaf_size / 2,
        bounding_box_origin.Y() + leaf_size / 2,
        bounding_box_origin.Z() - bounding_box_lengths.Z() / 2 +
        leaf_size / 2),
      bounding_box_origin, bounding_box_lengths, leaf_size);

    octomap_->prune();
    octomap_->updateInnerOccupancy();

    // set unknown to filled
    for (double x =
      leaf_size / 2 + bounding_box_origin.X() - bounding_box_lengths.X() / 2;
      x < bounding_box_origin.X() + bounding_box_lengths.X() / 2; x += leaf_size)
    {
      int progress =
        round(
        100 * (x + bounding_box_lengths.X() / 2 - bounding_box_origin.X()) /
        bounding_box_lengths.X());
      std::cout << "\rFilling closed spaces... " << progress << "%              ";

      for (double y =
        leaf_size / 2 + bounding_box_origin.Y() - bounding_box_lengths.Y() / 2;
        y < bounding_box_origin.Y() + bounding_box_lengths.Y() / 2;
        y += leaf_size)
      {
        for (double z = leaf_size / 2 + bounding_box_origin.Z() -
          bounding_box_lengths.Z() / 2;
          z < bounding_box_origin.Z() + bounding_box_lengths.Z() / 2;
          z += leaf_size)
        {
          octomap::OcTreeNode * seed = octomap_->search(x, y, z);
          if (!seed) {octomap_->setNodeValue(x, y, z, 1);}
        }
      }
    }

    octomap_->prune();
    octomap_->updateInnerOccupancy();

    std::cout << "\rOctomap generation completed                  " << std::endl;
  }

// Register this plugin with the simulator
  GZ_REGISTER_WORLD_PLUGIN(OctomapFromGazeboWorld)

}  // namespace gazebo
