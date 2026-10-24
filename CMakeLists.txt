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

add_executable(my_renderer src/my_renderer.cpp)
target_link_libraries(my_renderer PRIVATE renderer_core)

find_package(SDL2)
if(SDL2_FOUND)
    add_executable(my_viewer src/viewer.cpp)
    target_link_libraries(my_viewer PRIVATE renderer_core SDL2::SDL2)
else()
    message(STATUS "SDL2 not found, my_viewer will not be built")
endif()

# cmake --build build --target video  ->  renders frames and encodes src/out.mp4
set(MODELS ${CMAKE_SOURCE_DIR}/obj/diablo3_pose/diablo3_pose.obj CACHE STRING "Models to render")
add_custom_target(video
    COMMAND ${CMAKE_COMMAND} -E make_directory frames
    COMMAND $<TARGET_FILE:my_renderer> ${MODELS}
    COMMAND ffmpeg -y -loglevel error -framerate 30 -i frames/frame_%03d.tga -c:v libx264 -pix_fmt yuv420p out.mp4
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
    DEPENDS my_renderer
    VERBATIM
)
