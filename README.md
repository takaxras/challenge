# C++ Coding Challenge Solution

This project implements a simple optical flow and circle detection on live camera feed.

Given the time constraints, only basic optimizations were considered. No profiling has been done. No exception handling. Code refactoring would be desirable for improved readability and maintainability.

Boost.Interprocess (with shared memory) was initially considered as IPC mechanism, though the current chosen solution only makes use of std::async, as it is a very compact and efficient way of distributing computation tasks within a process.
The cost of copying frames across processes rather than a shared memory solution was weighted during the implementation, and it was considered to be negligible given modern architectural optimizations. Targeted profiling should be performed to compare these two solutions (and possibly others).

## Dependencies

The project makes use of modern C++ standard library classes and functions.
The only external dependency is OpenCV, used for camera capture and CV functions. Tested on OpenCV 4.10.0. See installation info below.

## Setting up working environment

This repo includes both configurations for building with either VS Code or CMake.
Development and testing platforms: Ubuntu VM running on WSL, and Linux Mint 21.2.

### Optional: Setup VS Code in WSL

https://code.visualstudio.com/docs/cpp/config-wsl

### Optional: Setup C++ in WSL

https://largecats.github.io/blog/2019/09/22/run-c-from-wsl-in-vs-code/

### Required: Setup OpenCV

An installation tutorial can be found here:
https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html

Follow guide from (copied locally as README_OpenCV_WSL.md) to ensure installing all necessary libraries:
https://github.com/Eemilp/install-opencv-on-wsl?tab=readme-ov-file

Notes on the guide above:
Step 3 - git
  - Change 'git checkout 4.5.1' commands to latest version branch (e.g. 4.10.0)
Step 3 - cmake changes:
  - avoid system-wide installation: CMAKE_INSTALL_PREFIX=$HOME/.local
  - use all cores (12 in this example)
cmake -DCMAKE_BUILD_TYPE=RELEASE -D OPENCV_GENERATE_PKGCONFIG=ON -DOPENCV_ENABLE_NONFREE=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DBUILD_opencv_legacy=OFF -DCMAKE_INSTALL_PREFIX=$HOME/.local ../opencv-4.x
make -j12 #increasing the number will make building faster. Maximum value can be found by running nproc.

Step 4 (VS Code specific)
  - Add includes in 'c_cpp_properties.json' ("includePath": ["/usr/local/include/opencv4"])
  - g++ failed unwrapping 'pkg-config --cflags --libs opencv4' argument in VS Code tasks.json.
    Fell back to manual arguments list

### Optional: OpenCV simple UI lib

The cvui lib (https://github.com/Dovyski/cvui) was considered as a simple solution for an interactive UI, but was not prioritized.

Downloaded cvui.h and added to src for reference.

### Deprecated: Boost

sudo apt-get install libboost-all-dev
  - Add includes in 'c_cpp_properties.json' ("includePath": ["/usr/include/boost"])

### Optional: Building with CMake

cmake -B build
cmake --build build
./build/challenge
