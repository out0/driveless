DOCKER_IMAGE_NAME=carlasim-dev-img

nvidia-docker run -it \
    --rm --privileged \
    --gpus all \
    -e NVIDIA_VISIBLE_DEVICES=0 \
    -e SDL_HINT_CUDA_DEVICE='0' \
    --net host \
    --user carla \
    --env DISPLAY=$DISPLAY \
    --env QT_X11_NO_MITSHM=1 \
    --env PULSE_SERVER=unix:/run/user/1000/pulse/native \
    --volume /tmp/.X11-unix:/tmp/.X11-unix \
    --runtime nvidia    \
    --expose 2000-2002 \
    $DOCKER_IMAGE_NAME \
    /bin/bash -c /home/carla/CarlaUE4.sh

