cmake_minimum_required(VERSION 2.8)
project(cf4ocl)

# The version number
set(cf4ocl_VERSION_MAJOR 1)
set(cf4ocl_VERSION_MINOR 3)
set(cf4ocl_VERSION_PATCH 0)

# Set executables prefix
set(EXECS_PREFIX "cf4ocl_")

# Compiler options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=gnu99")

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package (PkgConfig REQUIRED)
find_package(OpenCL REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)

# Library include directories
include_directories(${GLIB_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src/lib)

# Add src folder
add_subdirectory(src)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${cf4ocl_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${cf4ocl_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${cf4ocl_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include (CPack)


