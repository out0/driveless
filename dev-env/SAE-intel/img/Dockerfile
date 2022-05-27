#IMG-NAME: sae-ros-img
FROM ubuntu:20.04

RUN apt-get update

RUN apt-get install \
	python3 \
	python3-pip \
	sudo \
	libglu1-mesa-dev \
	xdg-user-dirs \
	pulseaudio \
	sudo \
	vulkan-utils \
	wget \
	libvulkan1 \
	mesa-vulkan-drivers \
	x11-xserver-utils \
    curl \
	-y --no-install-recommends

RUN pip3 install --upgrade pip

RUN pip3 install setuptools wheel

# Enable Vulkan support
ARG VULKAN_VERSION="1.2.198"

ENV NVIDIA_DRIVER_CAPABILITIES compute,graphics,utility,display

RUN apt-get install -y gnupg2

RUN export $(cat /etc/os-release | grep --color=never 'UBUNTU_CODENAME') && \
    wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | apt-key add - && \
    wget -qO /etc/apt/sources.list.d/lunarg-vulkan-${VULKAN_VERSION}-${UBUNTU_CODENAME}.list \
    https://packages.lunarg.com/vulkan/${VULKAN_VERSION}/lunarg-vulkan-${VULKAN_VERSION}-${UBUNTU_CODENAME}.list && \
    apt-get update && apt-get install -y --no-install-recommends vulkan-sdk && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN VULKAN_API_VERSION=`dpkg -s libvulkan1 | grep -oP 'Version: [0-9|\.]+' | grep -oP '[0-9|\.]+'` && \
    mkdir -p /etc/vulkan/icd.d/ && \
    echo \
    "{\
        \"file_format_version\" : \"1.0.0\",\
        \"ICD\": {\
            \"library_path\": \"libGLX_nvidia.so.0\",\
            \"api_version\" : \"${VULKAN_API_VERSION}\"\
        }\
    }" > /etc/vulkan/icd.d/nvidia_icd.json

# setup timezone
ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install packages
RUN apt-get update && apt-get install -q -y --no-install-recommends \
    dirmngr \
    gnupg2 \
    && rm -rf /var/lib/apt/lists/*

# setup keys
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

# ROS install
SHELL ["/bin/bash", "-c"]
RUN apt-get update && sudo apt install -y curl gnupg lsb-release
RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(source /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null
RUN apt-get update
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y ros-galactic-ros-base
RUN apt-get install -y python3-colcon-common-extensions \
  python3-flake8 \
  python3-pip \
  python3-pytest-cov \
  python3-rosdep \
  python3-setuptools \
  python3-vcstool
  
# setup environment
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash fsdssim_user && \ 
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \ 
	adduser fsdssim_user sudo && \ 
	adduser fsdssim_user audio && \ 
	adduser fsdssim_user video

USER fsdssim_user
WORKDIR /home/fsdssim_user
RUN sudo chown -R fsdssim_user /home/fsdssim_user

# Copy Simulator settings.json to Formula-Student-Driverless-Simulator Hard code from Simulator
# Please put your own settings.json if necessary

#RUN mkdir -p /home/fsdssim_user/Formula-Student-Driverless-Simulator/
#COPY settings.json /home/fsdssim_user/Formula-Student-Driverless-Simulator/


RUN wget -c https://github.com/FS-Driverless/Formula-Student-Driverless-Simulator/releases/download/v2.1.0/fsds-v2.1.0-linux.zip
RUN sudo apt-get update
RUN sudo apt-get dist-upgrade -y
RUN sudo apt-get install -y unzip git
RUN sudo apt-get install -y libyaml-cpp-dev libcurl4-openssl-dev
RUN echo 'alias rs="source /opt/ros/dashing/setup.bash"' >> /home/fsdssim_user/.bashrc

RUN unzip fsds-v2.1.0-linux.zip
RUN rm fsds-v2.1.0-linux.zip
RUN sudo apt-get install -y git

RUN git clone https://github.com/FS-Driverless/Formula-Student-Driverless-Simulator.git --recurse-submodules
WORKDIR /home/fsdssim_user/Formula-Student-Driverless-Simulator
RUN git checkout tags/v2.1.0

WORKDIR /home/fsdssim_user/Formula-Student-Driverless-Simulator/AirSim
RUN ./setup.sh




