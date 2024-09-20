# Use the ROS2 desktop VNC image as the base
FROM tiryoh/ros2-desktop-vnc:humble

# Maintainer information
LABEL maintainer="Fetullah Atas<fetulahatas1@gmail.com>"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies in one line, with apt-get update and cache clear
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep2 \
    python3-vcstool \
    xdotool \
    coinor-libipopt-dev \
    libfcl0.7 \
    pkg-config \
    libmosquitto-dev \
    && rm -rf /var/lib/apt/lists/*

# Create the ROS2 workspace and clone the VOX_NAV repository
RUN mkdir -p ~/ros2_ws/src
WORKDIR /root/ros2_ws

# Source ROS2 environment
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && rosdep update"

# Download underlay repositories for VOX_NAV and install dependencies
RUN wget https://raw.githubusercontent.com/jediofgever/vox_nav/humble/underlay.repos
RUN vcs import src < underlay.repos --recursive

# Install dependencies via rosdep, skip problematic keys, and ensure apt-get update is fresh
RUN apt-get update && rosdep install -y -r -q --from-paths src --ignore-src --rosdistro humble --skip-keys="cartographer-ros cartographer_ros" && rm -rf /var/lib/apt/lists/*

# Build ompl and casadi
RUN /bin/bash -c "source  /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select ompl casadi"

# # Copy ompl and casadi libraries to /usr/local/lib
# RUN cp install/ompl/lib/libompl.so* /usr/local/lib/ && \
#     cp install/casadi/lib/libcasadi.so* /usr/local/lib/

# Install ompl using apt-get. Somehow it is commented out in underlay.repos. Perhaps it is not necessary to build it - or its now unnecessary entirely?
RUN apt-get update && apt-get install -y libompl-dev && rm -rf /var/lib/apt/lists/*

# Install casadi using using cp
RUN cp -r install/casadi/include/casadi /usr/local/include/ && \
    cp install/casadi/lib/libcasadi.so* /usr/local/lib/

# Source the setup bash after the build
RUN /bin/bash -c "source install/setup.bash"

# Build all vox_nav packages except for vox_nav_control and vox_nav_misc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-skip-regex archive --packages-skip vox_nav_control vox_nav_misc"

# Source the setup bash after the build
RUN /bin/bash -c "source install/setup.bash"

# Install libmosquittopp-dev
RUN apt-get update && apt-get install -y libmosquittopp-dev && rm -rf /var/lib/apt/lists/*

# Build vox_nav_control
RUN /bin/bash -c "source build/ACADO/acado_env.sh && \
    source install/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select vox_nav_control"

# Optional: Build with TENSORRT_ROOT for lidar_apollo_instance_segmentation
# Uncomment the following section if required
# RUN colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=release \
#     -DACADOS_WITH_QPOASES=on -DACADO_CODE_IS_READY=on -DWITH_IPOPT=true \
#     -DTENSORRT_ROOT=/home/atas/downloads/tensorrt-8.4.1.5 \
#     -DHUMBLE_ROS=humble --packages-skip-regex archive --packages-skip vox_nav_control

RUN cd /tmp && curl -fOL https://github.com/cdr/code-server/releases/download/v4.92.2/code-server_4.92.2_amd64.deb && dpkg -i code-server_4.92.2_amd64.deb && rm code-server_4.92.2_amd64.deb
RUN bash -c 'echo -e "[supervisord]\nredirect_stderr=true\nstopsignal=QUIT\nautorestart=true\ndirectory=/root\n\n[program:codeserver]\ndirectory=/home/ubuntu\ncommand=/usr/bin/code-server --auth none --bind-addr 0.0.0.0:8888\nuser=ubuntu\nenvironment=DISPLAY=:1,HOME=/home/ubuntu,USER=ubuntu" > /etc/supervisor/conf.d/codeserver.conf'