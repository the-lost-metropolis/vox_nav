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
from launch.actions import (DeclareLaunchArgument, GroupAction,
                            IncludeLaunchDescription)
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import PushRosNamespace
from launch.conditions import IfCondition
import os
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():

    share_dir = get_package_share_directory('vox_nav_bringup')

    params = LaunchConfiguration('params')
    localization_params = LaunchConfiguration('localization_params')
    namespace = LaunchConfiguration('namespace')
    use_namespace = LaunchConfiguration('use_namespace')
    use_rviz = LaunchConfiguration('use_rviz')
    rviz_config = LaunchConfiguration('rviz_config')

    declare_params = DeclareLaunchArgument(
        'params',
        default_value=os.path.join(
            share_dir, 'params', 'vox_nav_default_params.yaml'),
        description='Path to the vox_nav parameters file.')

    declare_localization_params = DeclareLaunchArgument(
        'localization_params',
        default_value=os.path.join(
            share_dir, 'params', 'robot_localization_params.yaml'),
        description='Path to the localization parameters file.')

    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value='',
        description='Top-level namespace')

    declare_use_namespace_cmd = DeclareLaunchArgument(
        'use_namespace',
        default_value='False',
        description='Whether to apply a namespace to the vox_nav')

    declare_use_rviz_cmd = DeclareLaunchArgument(
        'use_rviz',
        default_value='False',
        description='Whether to use RVIZ')

    declare_rviz_config = DeclareLaunchArgument(
        'rviz_config',
        default_value=os.path.join(
            share_dir, 'rviz', 'vox_nav_default_view.yaml'),
        description='Path to the rviz config file.')

    # Specify the actions
    bringup_cmd_group = GroupAction([

        PushRosNamespace(
            condition=IfCondition(use_namespace),
            namespace=namespace),

        IncludeLaunchDescription(PythonLaunchDescriptionSource(
            os.path.join(share_dir, 'launch',
                         'vox_nav_core_nodes.launch.py')),
            launch_arguments={
            'params': params,
            'localization_params': localization_params
        }.items()),

        IncludeLaunchDescription(PythonLaunchDescriptionSource(
            os.path.join(share_dir, 'launch',
                                 'rviz.launch.py')),
                                 condition=IfCondition(use_rviz),
                                 launch_arguments={
            'use_namespace': use_namespace,
            'rviz_config': rviz_config
        }.items()),
    ])

    return LaunchDescription([
        declare_params,
        declare_localization_params,
        declare_use_rviz_cmd,
        declare_rviz_config,
        declare_namespace_cmd,
        declare_use_namespace_cmd,
        bringup_cmd_group
    ])
