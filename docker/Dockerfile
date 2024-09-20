# Use the ROS2 desktop VNC image as the base
FROM tiryoh/ros2-desktop-vnc:humble

# Maintainer information
LABEL maintainer="Fetullah Atas<fetulahatas1@gmail.com>"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Create the ubuntu user and its home directory
RUN useradd -m -d /home/ubuntu -s /bin/bash ubuntu && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install necessary dependencies in a single RUN command
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep2 \
    python3-vcstool \
    xdotool \
    coinor-libipopt-dev \
    libfcl0.7 \
    pkg-config \
    libmosquitto-dev \
    libmosquittopp-dev \
    libompl-dev \
    && rm -rf /var/lib/apt/lists/*

# Switch to user ubuntu
USER ubuntu

# Create the ROS2 workspace in ubuntu's home and clone the VOX_NAV repository
RUN mkdir -p /home/ubuntu/ros2_ws/src
WORKDIR /home/ubuntu/ros2_ws

# Source ROS2 environment
RUN echo "source /opt/ros/humble/setup.bash" >> /home/ubuntu/.bashrc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && rosdep update"

# Download underlay repositories for VOX_NAV and install dependencies
RUN wget https://raw.githubusercontent.com/jediofgever/vox_nav/humble/underlay.repos
RUN vcs import src < underlay.repos --recursive

# Install dependencies via rosdep, skip problematic keys
RUN sudo apt-get update && \
    sudo rosdep install -y -r -q --from-paths src --ignore-src --rosdistro humble --skip-keys="cartographer-ros cartographer_ros" && \
    sudo rm -rf /var/lib/apt/lists/*

# Build casadi (ompl is installed via apt-get)
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select casadi"

# Install casadi using cp
RUN sudo cp -r install/casadi/include/casadi /usr/local/include/ && \
    sudo cp install/casadi/lib/libcasadi.so* /usr/local/lib/

# Source the setup bash after the build
RUN /bin/bash -c "source install/setup.bash"

# Build all vox_nav packages except for vox_nav_control and vox_nav_misc
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-skip-regex archive --packages-skip vox_nav_control vox_nav_misc"

# Source the setup bash after the build
RUN /bin/bash -c "source install/setup.bash"

# Build vox_nav_control
RUN /bin/bash -c "source build/ACADO/acado_env.sh && \
    source install/setup.bash && \
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release \
    -DACADOS_WITH_QPOASES=ON -DACADO_CODE_IS_READY=ON -DWITH_IPOPT=true \
    --packages-select vox_nav_control"

# Install and configure code-server under ubuntu
RUN cd /tmp && curl -fOL https://github.com/cdr/code-server/releases/download/v4.92.2/code-server_4.92.2_amd64.deb && sudo dpkg -i code-server_4.92.2_amd64.deb && rm code-server_4.92.2_amd64.deb
RUN sudo bash -c 'echo -e "[supervisord]\nredirect_stderr=true\nstopsignal=QUIT\nautorestart=true\ndirectory=/home/ubuntu\n\n[program:codeserver]\ndirectory=/home/ubuntu/ros2_ws\ncommand=/usr/bin/code-server --auth none --bind-addr 0.0.0.0:8888 /home/ubuntu/ros2_ws\nuser=ubuntu\nenvironment=DISPLAY=:1,HOME=/home/ubuntu,USER=ubuntu" > /etc/supervisor/conf.d/codeserver.conf'

# Set the default user back to root (if needed for the runtime context)
USER root
