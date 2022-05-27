DOCKER_IMAGE_NAME=sae-ros-img

nvidia-docker run -it \
    --rm --privileged \
    --gpus all \
    --net host \
    --env DISPLAY=$DISPLAY \
    --env QT_X11_NO_MITSHM=1 \
    --env PULSE_SERVER=unix:/run/user/1000/pulse/native \
    --volume /tmp/.X11-unix:/tmp/.X11-unix \
    --volume /etc/alsa:/etc/alsa \
    --volume /usr/share/alsa:/usr/share/alsa \
    --volume /home/$USER/.config/pulse:/home/$USER/.config/pulse \
    --volume /run/user/1000/pulse/native:/run/user/1000/pulse/native \
    --volume /dev/snd:/dev/snd \
    $DOCKER_IMAGE_NAME \
    /bin/bash -c /home/fsdssim_user/FSDS.sh

