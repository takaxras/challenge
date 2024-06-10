cmake_minimum_required(VERSION 3.22)
project(challenge)

include_directories($ENV{HOME}/.local/include/opencv4)

link_directories($ENV{HOME}/.local/lib)

link_libraries(
                opencv_core
                opencv_video
                opencv_videoio
                opencv_imgproc
                opencv_highgui
                opencv_imgcodecs
                opencv_features2d
                )

add_executable(challenge src/main.cpp)
