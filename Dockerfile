# Use the ROS2 desktop VNC image as the base
FROM tiryoh/ros2-desktop-vnc:humble

##### STEP 1: Install all non-ros dependencies and environment setup

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Create the ubuntu user and its home directory
RUN useradd -m -d /home/ubuntu -s /bin/bash ubuntu && \
    echo "ubuntu:password" | chpasswd && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install necessary dependencies in a single RUN command
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep2 \
    xdotool \
    coinor-libipopt-dev \
    libfcl0.7 \
    pkg-config \
    libmosquitto-dev \
    libmosquittopp-dev \
    libompl-dev \
    ros-humble-twist-mux \
    ros-humble-navigation2 \
    && rm -rf /var/lib/apt/lists/*
# TODO: twist-mux and navigation2 should be installed via rosdep, try to remove them

# Switch to user ubuntu
USER ubuntu

##### STEP 2: We setup the ROS2 workspace and add the appropriate source code

# Create the ROS2 workspace in ubuntu's home and clone the VOX_NAV repository
RUN mkdir -p /home/ubuntu/ros2_ws/src
WORKDIR /home/ubuntu/ros2_ws

# Source ROS2 environment
RUN echo "source /opt/ros/humble/setup.bash" >> /home/ubuntu/.bashrc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && rosdep update"

# Clone casadi repository (main branch)
RUN git clone https://github.com/casadi/casadi.git src/casadi

# Clone acado repository (stable branch)
RUN git clone -b stable https://github.com/acado/acado.git src/acado

# COPY the current repository into the workspace as "vox_nav"
COPY . src/vox_nav

##### STEP 3: Now that all source code is in place, we install the dependenices with rosdep and build the workspace

# Install dependencies via rosdep, skip problematic keys
RUN sudo apt-get update && \
    sudo rosdep install -y -r -q --from-paths src --ignore-src --rosdistro humble --skip-keys="cartographer-ros cartographer_ros" && \
    sudo rm -rf /var/lib/apt/lists/*

# Build casadi
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select casadi"

# Install casadi using cp
RUN sudo cp -r install/casadi/include/casadi /usr/local/include/ && \
    sudo cp install/casadi/lib/libcasadi.so* /usr/local/lib/

# Build all vox_nav packages except for vox_nav_control and vox_nav_misc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-skip-regex archive --packages-skip vox_nav_control vox_nav_misc"

# Build vox_nav_control
RUN /bin/bash -c "source build/ACADO/acado_env.sh && \
    source install/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select vox_nav_control"

# Build "botanbot_bringup", "botanbot_description", "botanbot_gazebo", "botanbot_gui", "botanbot_navigation2"
RUN . /opt/ros/humble/setup.sh; \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release --packages-select ros2_full_sensor_suite botanbot_bringup botanbot_description botanbot_gazebo botanbot_gui botanbot_navigation2

##### STEP 4: We install and configure code-server

# Install and configure code-server under ubuntu
RUN cd /tmp && curl -fOL https://github.com/cdr/code-server/releases/download/v4.92.2/code-server_4.92.2_amd64.deb && sudo dpkg -i code-server_4.92.2_amd64.deb && rm code-server_4.92.2_amd64.deb
RUN sudo bash -c 'echo -e "[supervisord]\nredirect_stderr=true\nstopsignal=QUIT\nautorestart=true\ndirectory=/home/ubuntu\n\n[program:codeserver]\ndirectory=/home/ubuntu/ros2_ws\ncommand=/usr/bin/code-server --auth none --bind-addr 0.0.0.0:8888 /home/ubuntu/ros2_ws\nuser=ubuntu\nenvironment=DISPLAY=:1,HOME=/home/ubuntu,USER=ubuntu" > /etc/supervisor/conf.d/codeserver.conf'

# Set the default user back to root (if needed for the runtime context)
USER root
