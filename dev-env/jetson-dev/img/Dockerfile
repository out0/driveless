#IMG-NAME: jetson-l4t-img
FROM dustynv/jetson-inference:r34.1.1

RUN echo 'DPkg::options { "--force-confdef"; };' >> /etc/apt/apt.conf
RUN apt-get update
RUN apt-get install -y --no-install-recommends make g++
RUN apt-get install -y git cmake libpython3-dev python3-numpy sudo
RUN apt-get -y upgrade

RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash nvidia && \ 
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \ 
	adduser nvidia sudo && \ 
	adduser nvidia audio && \ 
	adduser nvidia video

RUN \
    echo "deb https://repo.download.nvidia.com/jetson/common r34.1 main" >> /etc/apt/sources.list && \
    echo "deb https://repo.download.nvidia.com/jetson/t194 r34.1 main" >> /etc/apt/sources.list && \
    apt-key adv --fetch-key http://repo.download.nvidia.com/jetson/jetson-ota-public.asc && \
    mkdir -p /opt/nvidia/l4t-packages/ && \
    touch /opt/nvidia/l4t-packages/.nv-l4t-disable-boot-fw-update-in-preinstall && \
    apt-get update && \
    apt-get install -y --no-install-recommends nvidia-l4t-core

RUN \
    apt-get install --no-install-recommends -y \
    nvidia-l4t-firmware \
    nvidia-l4t-multimedia-utils \
    nvidia-l4t-multimedia \
    nvidia-l4t-cuda \
    nvidia-l4t-x11 \
    nvidia-l4t-camera \
    nvidia-l4t-tools \
    nvidia-l4t-graphics-demos \
    nvidia-l4t-gstreamer \
    nvidia-l4t-jetson-io \
    nvidia-l4t-configs \
    nvidia-l4t-3d-core \
    nvidia-l4t-oem-config

RUN echo " export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/aarch64-linux-gnu/tegra/" >> /etc/profile


