cmake_minimum_required(VERSION 2.8)
project(cf4ocl)

# The version number
set(cf4ocl_VERSION_MAJOR 1)
set(cf4ocl_VERSION_MINOR 3)
set(cf4ocl_VERSION_PATCH 0)

# Compiler options
set(CMAKE_C_FLAGS "-Wextra -Wall -g -std=gnu99")

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package (PkgConfig REQUIRED)
find_package(OpenCL REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)

# Library include directories
include_directories(${GLIB_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src/lib)

# Add src folder
add_subdirectory(src)



