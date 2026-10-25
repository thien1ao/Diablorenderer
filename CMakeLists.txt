cmake_minimum_required(VERSION 3.16)
project(tinyrenderer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

add_library(renderer_core STATIC
    src/my_model.cpp
    src/pipeline.cpp
    src/tgaimage.cpp
)
target_compile_options(renderer_core PUBLIC -march=native)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(renderer_core PUBLIC OpenMP::OpenMP_CXX)
endif()

find_package(SDL2 REQUIRED)
add_executable(my_viewer src/viewer.cpp)
target_link_libraries(my_viewer PRIVATE renderer_core SDL2::SDL2)
