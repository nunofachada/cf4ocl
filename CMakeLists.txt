cmake_minimum_required(VERSION 2.8)
project(cf4ocl2)

# Set modules directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Find git
find_package(Git REQUIRED)
# Determine version from Git
include(GetVersionFromGitTag)

# Set log domain  
add_definitions(-DG_LOG_DOMAIN=\"${PROJECT_NAME}\")

# Set executables prefix
set(EXECS_PREFIX "ccl_")

# Compiler options for GCC
# -Wno-comment because of comment within comment in OpenCL headers
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -Wno-comment -std=c99")
endif()

# Find required libraries
find_package(PkgConfig REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Doxygen 1.8.3 QUIET)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)

# Use provided OpenCL headers or system OpenCL headers?
option(USE_LOCAL_OPENCL_HEADERS "Use local OpenCL headers?" OFF)

SET(LOCAL_OPENCL_HEADERS "1.2" CACHE STRING "OpenCL header version selected at CMake configure time")

SET_PROPERTY(CACHE LOCAL_OPENCL_HEADERS PROPERTY STRINGS "1.0" "1.1" "1.2" "2.0") 

IF (USE_LOCAL_OPENCL_HEADERS)
	SET(OPENCL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ocl/${LOCAL_OPENCL_HEADERS}")
ENDIF()

# Library include directories
include_directories(${GLIB_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} 
	${CMAKE_SOURCE_DIR}/src/${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

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
		# Add tests path to doc exclude path
		set(DOC_OPENCL_FUNCTION_URLS_FILTER "${PROJECT_SOURCE_DIR}/auxbuild/docfilter.sh")
	endif()
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
	if (${DOC_INSTALL})
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/ 
			DESTINATION ${INSTALL_SUBDIR_DOCS_HTML} OPTIONAL)
	endif()

endif()

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include (CPack)


