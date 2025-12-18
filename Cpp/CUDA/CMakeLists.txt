cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Release)
# 有编译器没驱动:)
set(CMAKE_CUDA_COMPILER /usr/local/cuda-12.5/bin/nvcc)
enable_language(CUDA)


project(hellocuda LANGUAGES CXX CUDA)
add_executable(main main.cu)