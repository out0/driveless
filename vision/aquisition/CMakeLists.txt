# require CMake 2.8 or greater
cmake_minimum_required(VERSION 2.8)

# declare my-recognition project
project(usb-camera-hello-world)

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

# add directory for libnvbuf-utils to program
link_directories(/usr/lib/aarch64-linux-gnu/tegra)

# compile the my-recognition program
SET(TARGET_H
    camera_drivers/source_camera.h
    camera_drivers/source_camera_gst.h
)
    

cuda_add_executable(usb2file test_usb_to_file.cpp)

cuda_add_executable(usbview test_usb_viewer.cpp)

# link my-recognition to jetson-inference library
target_link_libraries(usb2file jetson-inference ${OpenCV_LIBS})
target_link_libraries(usbview jetson-inference ${OpenCV_LIBS})