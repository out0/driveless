FROM ubuntu:20.04

SHELL ["/bin/bash", "-c"]
ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get install -y apt-utils
RUN apt-get dist-upgrade -y

# ROS install
RUN apt-get install -y curl

RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(source /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null
RUN apt-get update
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y ros-galactic-ros-base 

RUN apt-get install -y \
  build-essential \
  cmake \
  git \
  python3-colcon-common-extensions \
  python3-flake8 \
  python3-pip \
  python3-pytest-cov \
  python3-rosdep \
  python3-setuptools \
  python3-vcstool \
  wget


## Install OpenCV
##-------------------------------
RUN mkdir -p /tf
RUN cd /tf && git clone https://github.com/opencv/opencv.git
RUN cd /tf && git clone https://github.com/opencv/opencv_contrib.git
RUN mkdir -p /tf/opencv/build

RUN apt-get install -y build-essential cmake pkg-config libgtk-3-dev \
    libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
    gfortran openexr libatlas-base-dev python3-dev python3-numpy \
    libtbb2 libtbb-dev libdc1394-22-dev libopenexr-dev \
    libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev \
    sudo

#RUN cd ~/opencv_build/opencv
RUN cd /tf/opencv/build && cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=ON \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D OPENCV_GENERATE_PKGCONFIG=ON \
    -D OPENCV_EXTRA_MODULES_PATH=/tf/opencv_contrib/modules \
    -D BUILD_EXAMPLES=ON ..

RUN cd /tf/opencv/build && make -j12
RUN cd /tf/opencv/build && make install

##-------------------------------

# V4l2 (Video for linux utils, for debugging)
# try: $ v4l2-ctl -d /dev/video0 --list-formats
RUN apt-get install -y v4l-utils \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-plugins-bad

# OMPL install
RUN apt-get install -y libompl-dev ompl-demos


RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash -u 1000 out0 && \ 
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \ 
	adduser out0 sudo && \ 
	adduser out0 audio && \ 
	adduser out0 video


ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

RUN echo 'alias rs="source /opt/ros/galactic/setup.bash"' >> /home/out0/.bashrc