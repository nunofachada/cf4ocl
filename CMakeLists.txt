cmake_minimum_required(VERSION 2.8)
project(cf4ocl2)

# Set modules directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Find git
find_package(Git QUIET)

# Determine version from Git
include(GetVersionFromGitTag)

# Required for building with Visual Studio
include(GenerateExportHeader)

# Set log domain
add_definitions(-DG_LOG_DOMAIN=\"${PROJECT_NAME}\")

# Set executables prefix
set(EXECS_PREFIX "ccl_")

# Compiler options for GCC/Clang
# -Wno-comment because of comment within comment in OpenCL headers
if((${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	OR (${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))
	# Clang and GCC flags
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -Wno-comment -std=c99")
	# Search for glib if using clang or GCC
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)
elseif(${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
	# PkgConfig doesnt work well with Visual Studio, so make user
	# specify location of GLib stuff
	find_library(GLIB_LIBRARIES glib-2.0)
	find_path(GLIB_INCLUDE_DIR glib.h)
	find_path(GLIB_CONFIG_INCLUDE_DIR glibconfig.h)
	set(GLIB_INCLUDE_DIRS "${GLIB_INCLUDE_DIR}" "${GLIB_CONFIG_INCLUDE_DIR}")
	# This line is required for glib to work with MSVC
	add_definitions("-Dinline=__inline")
	# And this avoids some warnings
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

# Find required libraries
find_package(OpenCL REQUIRED)
find_package(Doxygen 1.8.3 QUIET)

# Use provided OpenCL headers or system OpenCL headers?
if (NOT APPLE) # Not for Apple, which handles these headers differently

	option(USE_LOCAL_OPENCL_HEADERS "Use local OpenCL headers?" OFF)

	SET(LOCAL_OPENCL_HEADERS "1.2" CACHE STRING "OpenCL header version selected at CMake configure time")

	SET_PROPERTY(CACHE LOCAL_OPENCL_HEADERS PROPERTY STRINGS "1.0" "1.1" "1.2" "2.0")

	if (USE_LOCAL_OPENCL_HEADERS)
		SET(OPENCL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ocl/${LOCAL_OPENCL_HEADERS}")
	endif()

endif()

# Library include directories
include_directories(${GLIB_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src/${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
	${CMAKE_BINARY_DIR}/src/${PROJECT_NAME})

# Define install subdirs
set(INSTALL_SUBDIR_DOCS_HTML share/doc/${PROJECT_NAME})
set(INSTALL_SUBDIR_INCLUDE include)
set(INSTALL_SUBDIR_BIN bin)
set(INSTALL_SUBDIR_LIB lib)

# Add src folder
add_subdirectory(src)

# Add scripts folder
add_subdirectory(scripts)

# add a target to generate API documentation with Doxygen
if(DOXYGEN_FOUND)

	# Set doc directory
	set(DOCS_DIRECTORY "doc")

	# Install docs?
	option(DOC_INSTALL "Install API documentation?" OFF)

	# Create links to OpenCL API when OpenCL functions are mentioned?
	# This is slow and requires Bash and other command-line utilties
	option(DOC_OPENCL_FUNCTION_URLS "Create links to OpenCL API (much slower)?" OFF)
	if (DOC_OPENCL_FUNCTION_URLS)
		# Create those links
		set(DOC_OPENCL_FUNCTION_URLS_FILTER "${PROJECT_SOURCE_DIR}/auxbuild/docfilter.sh")
	endif()

	# Copy and configure doxigen configuration file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

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
	if (${DOC_INSTALL})
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/
			DESTINATION ${INSTALL_SUBDIR_DOCS_HTML} OPTIONAL)
	endif()

endif()

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A cross-platform pure C99 object-oriented framework for developing and benchmarking OpenCL projects in C/C++")
set(CPACK_RESOURCE_VENDOR "FakenMC")

if(WIN32)
	set(CPACK_GENERATOR "NSIS" "ZIP")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
	set(CPACK_NSIS_URL_INFO_ABOUT "http://fakenmc.github.io/cf4ocl/" )
elseif(APPLE)
elseif(UNIX)
	set(CPACK_GENERATOR "DEB;TGZ;TZ")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nuno Fachada <faken@fakenmc.com>")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0-0 (>= 2.32.1), ocl-icd-libopencl1 (>= 1.3)")
	set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ocl-icd-opencl-dev (>= 1.3), opencl-headers (>= 1.0)")
endif()

include (CPack)


