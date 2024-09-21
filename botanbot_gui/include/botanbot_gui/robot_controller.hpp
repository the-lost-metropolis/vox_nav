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

#ifndef BOTANBOT_GUI__ROBOT_CONTROLLER_HPP_
#define BOTANBOT_GUI__ROBOT_CONTROLLER_HPP_

// ROS2 headers
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/pose_stamped.h>
// TF2 headers

#include <vox_nav_msgs/action/navigate_to_pose.hpp>
// STL headers
#include <vector>
#include <memory>

namespace botanbot_gui
{
enum class ActionStatus
{
  UNKNOWN = 0,
  PROCESSING = 1,
  FAILED = 2,
  SUCCEEDED = 3
};

/**
 * @brief Controls robot actions through navigation stack
 *
 */
class RobotController
{
public:
  // type shorteneings
  using ClientT = vox_nav_msgs::action::NavigateToPose;
  using ActionClient = rclcpp_action::Client<ClientT>;
  using NavigationGoalHandle = rclcpp_action::ClientGoalHandle<ClientT>;

  /**
   * @brief Construct a new Robot Controller object
   *
   */
  RobotController();

  /**
   * @brief Destroy the Robot Controller object
   *
   */
  ~RobotController();

  /**
   * @brief send robot to passed pose
   *
   * @param pose goal pose
   */
  void toTargetPose(geometry_msgs::msg::PoseStamped pose);

  /**
   * @brief cancel any active goals
   *
   */
  void cancelGoals();

private:
  // client to navigate to pose action
  ActionClient::SharedPtr nav_to_pose_client_;
  // rclcpp node
  rclcpp::Node::SharedPtr node_;

};
}  // namespace botanbot_gui

#endif  // BOTANBOT_GUI__ROBOT_CONTROLLER_HPP_
