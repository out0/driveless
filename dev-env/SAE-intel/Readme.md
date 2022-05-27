# Development Environment for SAE driveless

This is a dev-env for testing/developing code for the driveless project in a docker container.
This container contains:
    - The SAE simulator
    - ROS2 Galactic
    - CUDA enabled OS

##  Cuda version

This will create a CUDA based version container for running the SAE simulator in docker


##  Files

- build.sh  - builds the docker image
- run.sh    - starts the simulator

In the container you will find that:

- fsdssim_user is the simulator user
- fsdssim_user is in sudoers
- /home/fsdssim_user will contain the simulator files
- /home/fsdssim_user/FSDS.sh will run the simulator.


##  Run in Visual Studio Code

It is possible to use this container as a dev enviroment by opening it in VSCode with the remote containers extension
"ms-vscode-remote.remote-containers"

Just open SAE-cuda/ and chose "reopen in container".

Please check the .devcontainer/devcontainer.json  file

