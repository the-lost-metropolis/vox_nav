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
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node


GAZEBO_WORLD = os.environ['GAZEBO_WORLD']


def generate_launch_description():

    botanbot_bringup_dir = get_package_share_directory('botanbot_bringup')

    joy_config_filepath = LaunchConfiguration(
        'config_filepath', default=os.path.join(botanbot_bringup_dir, 'params', 'joystick_xbox.yaml'))

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    world_file_name = GAZEBO_WORLD + '/' + GAZEBO_WORLD + '.world'
    world = os.path.join(get_package_share_directory('botanbot_gazebo'),
                         'worlds', world_file_name)
    launch_file_dir = os.path.join(
        get_package_share_directory('botanbot_gazebo'), 'launch')

    return LaunchDescription([

        ExecuteProcess(cmd=[
            'gazebo', '--verbose', world, '-s', 'libgazebo_ros_factory.so'
        ],
            output='screen'),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                [launch_file_dir, '/robot_state_publisher.launch.py']),
            launch_arguments={'use_sim_time': use_sim_time}.items(),
        ),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                [os.path.join(get_package_share_directory('teleop_twist_joy'), 'launch', 'teleop-launch.py')]),
            launch_arguments={
                'config_filepath': joy_config_filepath}.items()
        ),

        IncludeLaunchDescription(PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('twist_mux'), 'launch', 'twist_mux_launch.py')),
        )
    ])
