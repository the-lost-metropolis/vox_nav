# Copyright (c) 2020 Fetullah Atas, Norwegian University of Life Sciences
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import GroupAction
from launch.actions import DeclareLaunchArgument

import os


def generate_launch_description():
    share_dir = get_package_share_directory("vox_nav_map_server")

    params = LaunchConfiguration("params")
    localization_params = LaunchConfiguration("localization_params")
    use_sim_time = LaunchConfiguration("use_sim_time")

    declare_params = DeclareLaunchArgument(
        "params",
        default_value=os.path.join(share_dir, "config", "osm_map.yaml"),
        description="Path to the vox_nav parameters file.",
    )

    declare_localization_params = DeclareLaunchArgument(
        "localization_params",
        default_value=os.path.join(share_dir, "config", "rl_sim.yaml"),
        description="Path to the localization parameters file.",
    )

    map_server_node = Node(
        package="vox_nav_map_server",
        executable="map_manager",
        name="vox_nav_map_server_rclcpp_node",
        namespace="",
        output="screen",
        # prefix=['xterm -e gdb -ex run --args'],
        parameters=[params],
    )

    urdf_file_name = "dobbie.urdf"
    urdf = os.path.join(
        get_package_share_directory("vox_nav_map_server"), "urdf", urdf_file_name
    )

    print("urdf_file_name : {}".format(urdf))

    declare_use_sim_time = DeclareLaunchArgument(
        "use_sim_time",
        default_value="true",
        description="Use simulation (Gazebo) clock if true",
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"use_sim_time": use_sim_time}],
        arguments=[urdf],
    )

    osm_map_node = Node(
        package="vox_nav_map_server",
        executable="osm_map_manager",
        name="osm_map_manager_rclcpp_node",
        output="screen",
        # prefix=["xterm -e gdb -ex run --args"],
        parameters=[params],
    )
    planner_server_node = Node(
        package="vox_nav_planning",
        executable="planner_server",
        name="vox_nav_planner_server_rclcpp_node",
        namespace="",
        output="screen",
        prefix=["xterm -e gdb -ex run --args"],
        parameters=[params],
    )
    controller_server_node = Node(
        package="vox_nav_control",
        executable="vox_nav_controller_server",
        name="vox_nav_controller_server_rclcpp_node",
        namespace="",
        output="screen",
        prefix=["xterm -e gdb -ex run --args"],
        parameters=[params],
    )
    navigate_to_pose_server_node = Node(
        package="vox_nav_navigators",
        executable="navigate_to_pose_server_node",
        name="navigate_to_pose_server_node",
        namespace="",
        output="screen",
        # prefix=['xterm -e gdb -ex run --args'],
        parameters=[params],
    )
    ekf_local_filter_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_local_filter_node",
        # output='screen',
        output={"both": "log"},
        parameters=[localization_params],
        remappings=[("odometry/filtered", "odometry/local")],
    )

    ekf_global_filter_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_global_filter_node",
        # output='screen',
        output={"both": "log"},
        parameters=[localization_params],
        remappings=[("odometry/filtered", "odometry/global")],
    )

    navsat_transform_node = Node(
        package="robot_localization",
        executable="navsat_transform_node",
        name="navsat_transform_node",
        # change it to screen if you wanna see RVIZ output in terminal
        # output={'both': 'log'},
        output="screen",
        parameters=[localization_params],
        remappings=[
            ("imu", "AGV0/dobbie/sensing/imu/tamagawa/imu_raw"),
            ("gps/fix", "AGV0/fix"),
            ("gps/filtered", "gps/filtered"),
            ("odometry/gps", "odometry/gps"),
            ("odometry/filtered", "odometry/global"),
        ],
    )

    return LaunchDescription(
        [
            declare_use_sim_time,
            declare_localization_params,
            declare_params,
            robot_state_publisher,
            osm_map_node,
            planner_server_node,
            controller_server_node,
            navigate_to_pose_server_node,
            ekf_local_filter_node,
            ekf_global_filter_node,
            navsat_transform_node,
        ]
    )
