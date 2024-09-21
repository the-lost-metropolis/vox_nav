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

#include <memory>
#include "botanbot_gui/robot_controller.hpp"

namespace botanbot_gui
{
  RobotController::RobotController(/* args */)
  {
    node_ = rclcpp::Node::make_shared("robot_controller_rclcpp_node");
    node_->set_parameter(rclcpp::Parameter("use_sim_time", true));

    nav_to_pose_client_ =
      rclcpp_action::create_client<ClientT>(
      node_, "navigate_to_pose");

    rclcpp::Clock::SharedPtr
      clock = std::make_shared<rclcpp::Clock>(RCL_SYSTEM_TIME);

    RCLCPP_INFO(node_->get_logger(), "Creating Robot Controller Instance");
  }

  RobotController::~RobotController()
  {
    RCLCPP_INFO(node_->get_logger(), "Destroying A Robot Controller Instance");
  }

  void RobotController::toTargetPose(geometry_msgs::msg::PoseStamped pose)
  {
    auto is_action_server_ready =
      nav_to_pose_client_->wait_for_action_server(std::chrono::seconds(5));
    // cancel any other goals
    cancelGoals();
    if (!is_action_server_ready) {
      RCLCPP_ERROR(
        node_->get_logger(), "navigate_to_pose action server is not available."
      );
      return;
    }

    vox_nav_msgs::action::NavigateToPose::Goal navigation_goal;
    navigation_goal.pose = pose;

    // Enable result awareness by providing an empty lambda function
    auto send_goal_options =
      rclcpp_action::Client<ClientT>::SendGoalOptions();
    send_goal_options.result_callback = [](auto) {};

    auto future_goal_handle =
      nav_to_pose_client_->async_send_goal(navigation_goal, send_goal_options);

    if (rclcpp::spin_until_future_complete(
        node_,
        future_goal_handle) != rclcpp::FutureReturnCode::SUCCESS)
    {
      RCLCPP_ERROR(node_->get_logger(), "Send goal call failed");
      return;
    }

    // Get the goal handle and save so that we can check on completion in the timer callback
    NavigationGoalHandle::SharedPtr navigation_goal_handle = future_goal_handle.get();

    auto result_future = nav_to_pose_client_->async_get_result(navigation_goal_handle);

    rclcpp_action::ClientGoalHandle<ClientT>::WrappedResult wrapped_result =
      result_future.get();

    if (!navigation_goal_handle) {
      RCLCPP_ERROR(node_->get_logger(), "Goal was rejected by server");
      return;
    }
  }

  void RobotController::cancelGoals()
  {
    auto future_cancel = nav_to_pose_client_->async_cancel_all_goals();
    if (rclcpp::spin_until_future_complete(
        node_,
        future_cancel) != rclcpp::FutureReturnCode::SUCCESS)
    {
      RCLCPP_ERROR(node_->get_logger(), "Failed to cancel goal");
      return;
    }
  }
}  // namespace botanbot_gui
