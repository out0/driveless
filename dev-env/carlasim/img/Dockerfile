#IMG-NAME: carlasim-dev-img
FROM carlasim/carla:latest

USER root
SHELL ["/bin/bash", "-c"]

ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN rm /etc/apt/sources.list.d/*
RUN apt-key del 7fa2af80
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/3bf863cc.pub

RUN apt-get update
RUN apt-get install -y apt-utils software-properties-common
RUN apt-get dist-upgrade -y
RUN apt-get install -y python3-pip
RUN apt-get install -y libjpeg-turbo8 libtiff5
RUN apt-get install -y sudo curl
RUN adduser carla sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN pip3 install carla
RUN export DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY

# ROS install
# RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
# RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(source /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null
# RUN apt-get update
# ENV DEBIAN_FRONTEND=noninteractive
# RUN apt-get install -y ros-dashing-desktop
# RUN apt-get install -y libvulkan1 mesa-vulkan-drivers vulkan-utils
