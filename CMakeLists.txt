cmake_minimum_required(VERSION 3.15...3.30)

project(ctext DESCRIPTION "Cellular Textures")

set(CTEXT_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/include"
    CACHE STRING "${PROJECT_NAME} include directory.")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(set_compile_link_flags)

add_subdirectory(apps)
add_subdirectory(src)
