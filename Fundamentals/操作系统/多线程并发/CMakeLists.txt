cmake_minimum_required(VERSION 3.22)
project(PTHREAD CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(threadtest thread.cc)

find_package(Threads REQUIRED)
target_link_libraries(threadtest PUBLIC Threads::Threads)