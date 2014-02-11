cmake_minimum_required(VERSION 2.8)
project(cf4ocl)

# The version number
set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 3)
set(${PROJECT_NAME}_VERSION_PATCH 0)

# Set executables prefix
set(EXECS_PREFIX "${PROJECT_NAME}_")

# Compiler options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=gnu99")

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(PkgConfig REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Doxygen 1.8.3 QUIET)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)

# Library include directories
include_directories(${GLIB_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src/lib)

# Add src folder
add_subdirectory(src)

# Add scripts folder
add_subdirectory(scripts)

# add a target to generate API documentation with Doxygen
if(DOXYGEN_FOUND)
	set(DOCS_DIRECTORY "doc")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	# Convert GitHub README.md into Doxygen MD format
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/README.md README_CONTENTS)
	string(REPLACE "```c" "~~~~~~~~~~~~~~~{.c}" README_CONTENTS_MOD1 ${README_CONTENTS})
	string(REPLACE "```" "~~~~~~~~~~~~~~~" README_CONTENTS_MOD2 ${README_CONTENTS_MOD1})
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/README.doxy.md ${README_CONTENTS_MOD2})
	# Add a build docs target
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
	# Copy images to doc directory 
	add_custom_command(TARGET doc POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/images 
		${DOCS_DIRECTORY}/html/images
	)
	# Add documentation installation target (if the docs have been built)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/ DESTINATION share/${PROJECT_NAME} OPTIONAL)
endif()

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include (CPack)


