#!/bin/bash
current_dir="$(dirname "${BASH_SOURCE[0]}")"  # get the directory name
current_dir="$(realpath "${current_dir}")"    # resolve its full path if
source /opt/ros/humble/setup.bash
source $current_dir/../../../install/setup.bash
source ~/.bashrc
echo "going to shut down any active gzserver before starting ..."
killall gzserver
export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH$current_dir/../../../src/botanbot_sim/botanbot_gazebo/models
export GAZEBO_MODEL_PATH=$current_dir/../../../src/botanbot_sim/ros2_full_sensor_suite/models:$GAZEBO_MODEL_PATH
export GAZEBO_WORLD=$GAZEBO_WORLD$1
ros2_launch_command="ros2 launch botanbot_bringup botanbot_simulation.launch.py; bash"
gnome-terminal -- sh -c "$ros2_launch_command"
echo "executing command: $ros2_launch_command"
echo "selected world is: $GAZEBO_WORLD"