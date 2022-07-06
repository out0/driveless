# require CMake 2.8 or greater
cmake_minimum_required(VERSION 2.8)

# declare my-recognition project
project(driveless-vision-module)

# import jetson-inference and jetson-utils packages.
# note that if you didn't do "sudo make install"
# while building jetson-inference, this will error.
find_package(jetson-utils REQUIRED)
find_package(jetson-inference REQUIRED)
find_package(OpenCV REQUIRED)

# CUDA is required
find_package(CUDA)

message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")

set(CMAKE_CXX_STANDARD 14)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/)

link_directories(/usr/lib/aarch64-linux-gnu/tegra)

SET(TARGET_H
    model/vision_formats.h
    acquisition/source_camera.h
    acquisition/source_camera_usb.h
    control/process_handler.h
    occupancy_grid/occupancy_grid.h
    log/logger.h
    segmentation/neuralnet_segmentation.h
    communication/stream_server.h
)

cuda_add_executable(
    vision 
    vision.cpp 
    occupancy_grid/occupancy_grid_impl.cpp 
    control/process_handler_impl.cpp 
    log/logger_debug_stdout.cpp 
    segmentation/neuralnet_segmentation.cpp
    communication/stream_server.cpp
)

# link my-recognition to jetson-inference library
target_link_libraries(vision jetson-inference ${OpenCV_LIBS})