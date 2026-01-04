cmake_minimum_required(VERSION 3.18)

if (NOT CMKAE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)

project(qc_base LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Werror=return-type)


file(GLOB sources CONFIGURE_DEPENDS *.cc)
foreach(source IN ITEMS ${sources})
    get_filename_component(name "${source}" NAME_WLE)
    add_executable(${name} ${name}.cc)
endforeach()