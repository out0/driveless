# require CMake 2.8 or greater
cmake_minimum_required(VERSION 2.8)

project(stream-test)

# import jetson-inference and jetson-utils packages.
# note that if you didn't do "sudo make install"
# while building jetson-inference, this will error.

message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")

set(CMAKE_CXX_STANDARD 14)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/)

link_directories(/usr/lib/aarch64-linux-gnu/tegra)

SET(TARGET_H
    ../../log/logger.h
    ../../streaming/stream_client.h
)

add_executable(
    stream_tst
    test_receive_streams.cpp
    ../../log/logger_debug_stdout.cpp
    ../../streaming/stream_client.cpp
)

# link my-recognition to jetson-inference library
target_link_libraries(stream_tst)