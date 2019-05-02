cmake_minimum_required(VERSION 2.8.3)
project(cf4ocl2)

# Set modules directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Set log domain
add_definitions(-DG_LOG_DOMAIN=\"${PROJECT_NAME}\")

# #################### #
# DEPENDENCIES SECTION #
# #################### #

# Find git
find_package(Git QUIET)
# Update version from Git?
if (GIT_FOUND)
	option(VERSION_UPDATE_FROM_GIT "Update project version from git?" OFF)
endif()
# Determine version from Git
include(GetVersionFromGitTag)
# Required for building with Visual Studio
include(GenerateExportHeader)
# Auto-determine architecture
include(TargetArch)
# Include code coverage build type
include(CodeCoverage)

# Search for glib with pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32.1)

# Find OpenCL
find_package(OpenCL REQUIRED)
set(OpenCL_SYSTEM_INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS})

# Find optional executables for creating docs
find_package(Doxygen 1.8.3 QUIET)
find_package(LATEX QUIET)

# ################ #
# COMPILER OPTIONS #
# ################ #

# Compiler options for GCC/Clang
# -Wno-comment because of comment within comment in OpenCL headers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -Wno-comment -std=c99")

# Avoid including MinGW dll dependency
if(MINGW)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
endif()

# Definitions for release builds
if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "Release"))

	# Disable assertions on release builds
	add_definitions("-DG_DISABLE_ASSERT")

elseif((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))

	# Disable assertions (e.g. g_return_if_fail) on minimum size release builds
	add_definitions("-DG_DISABLE_ASSERT")

	# Disable checks on minimum size release builds
	add_definitions("-DG_DISABLE_CHECKS")

endif()

# ################################### #
# OPTIONALLY USE LOCAL OPENCL HEADERS #
# ################################### #

# Use provided OpenCL headers or system OpenCL headers?
if (NOT APPLE) # Not for Apple, which handles these headers differently

	option(OpenCL_USE_LOCAL_HEADERS "Use local OpenCL headers?" OFF)

	if (OpenCL_USE_LOCAL_HEADERS)

		set(OpenCL_LOCAL_HEADERS "1.2" CACHE STRING
			"OpenCL header version selected at CMake configure time")

		set_property(CACHE OpenCL_LOCAL_HEADERS PROPERTY STRINGS
			"1.0" "1.1" "1.2" "2.0" "2.1" "2.2")

		set(OpenCL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ocl")
		set(OpenCL_VERSION_STRING ${OpenCL_LOCAL_HEADERS})
		string(SUBSTRING ${OpenCL_VERSION_STRING} 0 1 OpenCL_VERSION_MAJOR)
		string(SUBSTRING ${OpenCL_VERSION_STRING} 2 1 OpenCL_VERSION_MINOR)

	else()

		unset(OpenCL_LOCAL_HEADERS CACHE)

	endif()

	add_definitions(-DCL_TARGET_OPENCL_VERSION=${OpenCL_VERSION_MAJOR}${OpenCL_VERSION_MINOR}0)

endif()

# ########################### #
# SET LIBRARY INCLUDE FOLDERS #
# ########################### #
include_directories(
	${GLIB_INCLUDE_DIRS}                 # GLib headers
	${OpenCL_INCLUDE_DIRS}               # OpenCL headers
	${CMAKE_BINARY_DIR}/include/cf4ocl2  # Library headers
	${CMAKE_BINARY_DIR}/include)         # Aggregate library header

# ###################### #
# DEFINE INSTALL FOLDERS #
# ###################### #
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
	set(CMAKE_INSTALL_BINDIR "bin"
		CACHE PATH "Install sub-location for executables")
endif(NOT DEFINED CMAKE_INSTALL_BINDIR)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
	set(CMAKE_INSTALL_LIBDIR "lib"
		CACHE PATH "Install sub-location for object code libraries")
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
	set(CMAKE_INSTALL_INCLUDEDIR "include"
		CACHE PATH "Install sub-location for header files")
endif(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)

if(NOT DEFINED CMAKE_INSTALL_DATADIR)
	set(CMAKE_INSTALL_DATADIR "share"
		CACHE PATH "Install sub-location for read-only data")
endif(NOT DEFINED CMAKE_INSTALL_DATADIR)

if(NOT DEFINED CMAKE_INSTALL_DOCDIR)
	set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc"
		CACHE PATH "Install sub-location for documentation")
endif(NOT DEFINED CMAKE_INSTALL_DOCDIR)

if(NOT DEFINED CMAKE_INSTALL_MANDIR)
	set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATADIR}/man"
		CACHE PATH "Install sub-location for man docs")
endif(NOT DEFINED CMAKE_INSTALL_MANDIR)

mark_as_advanced(CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR
	CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DATADIR
	CMAKE_INSTALL_DOCDIR CMAKE_INSTALL_MANDIR)

# ########################### #
# BUILD DOCUMENTATION SECTION #
# ########################### #

# add a target to generate API documentation with Doxygen
if(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND AND LATEX_COMPILER)

	# Doc build is enabled
	set(DOC_BUILD "ON" CACHE INTERNAL "Build documentation")

	# Set doc generation directory
	set(DOCS_DIRECTORY "doc")

	# Document generation options
	option(DOC_GENERATE_HTML "Generate HTML documentation?" ON)
	option(DOC_INTERNAL
		"Generate internal documentation (for cf4ocl development only)?" OFF)
	option(DOC_GENERATE_LATEX "Generate Latex documentation?" OFF)
	if (DOC_GENERATE_LATEX AND CMAKE_MAKE_PROGRAM
		AND PDFLATEX_COMPILER AND MAKEINDEX_COMPILER)
		option(DOC_GENERATE_PDF "Generate PDF documentation?" OFF)
	else()
		unset(DOC_GENERATE_PDF CACHE)
	endif()
	if(UNIX)
		option(DOC_GENERATE_MAN "Generate man pages?" OFF)
	endif()

	# Exclude tests from documentation?
	if (DOC_BUILD)
		option(DOC_EXCLUDE_TESTS "Exclude tests from API docs?" ON)
		if (DOC_EXCLUDE_TESTS)
			# Add tests path to doc exclude path
			set(DOX_EXCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/tests"
				CACHE INTERNAL "")
		endif()
	endif()

	# Set doxygen document generation options
	if (DOC_GENERATE_HTML)
		set(DOX_GENERATE_HTML "YES")
	else()
		set(DOX_GENERATE_HTML "NO")
	endif()
	if (DOC_GENERATE_LATEX)
		set(DOX_GENERATE_LATEX "YES")
	else()
		set(DOX_GENERATE_LATEX "NO")
	endif()
	if (DOC_GENERATE_MAN)
		set(DOX_GENERATE_MAN "YES")
	else()
		set(DOX_GENERATE_MAN "NO")
	endif()
	if (DOC_INTERNAL)
		set(DOX_INTERNAL "YES")
	else()
		set(DOX_INTERNAL "NO")
	endif()

	# Document install options
	if (DOC_GENERATE_HTML)
		option(DOC_INSTALL_HTML "Install HTML documentation?" OFF)
	else()
		unset(DOC_INSTALL_HTML CACHE)
	endif()
	if (DOC_GENERATE_PDF)
		option(DOC_INSTALL_PDF "Install Latex documentation?" OFF)
	else()
		unset(DOC_INSTALL_PDF CACHE)
	endif()
	if (DOC_GENERATE_MAN)
		option(DOC_INSTALL_MAN "Install man pages" OFF)
	else()
		unset(DOC_INSTALL_MAN CACHE)
	endif()

	# If git is found, set document file version filter to last
	# commit where file was changed
	if (GIT_FOUND)
		set(DOX_FILE_VERSION_FILTER
			"${GIT_EXECUTABLE} log -n 1 --pretty=%h --")
	endif()

	# Linux/OSX options
	if (UNIX)

		# Create links to OpenCL API when OpenCL functions are mentioned?
		# This is slow and requires Bash and other command-line utilties
		option(DOC_OPENCL_FUNCTION_URLS
			"Create links to OpenCL API (much slower)?" OFF)
		if (DOC_OPENCL_FUNCTION_URLS)
			# Create those links
			set(DOX_OPENCL_FUNCTION_URLS_FILTER
				"${PROJECT_SOURCE_DIR}/auxbuild/docfilter.sh")
		endif()

	endif()

	# Copy and configure doxigen configuration file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

	# Add a build docs target
	if (DOC_GENERATE_HTML OR DOC_GENERATE_LATEX OR DOC_GENERATE_MAN)
		add_custom_target(doc
			${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating documentation with Doxygen" VERBATIM)
	else()
		add_custom_target(doc
			${CMAKE_COMMAND} -E echo "Enable at least one document format."
			COMMENT "All document generation formats are disabled!" VERBATIM)
	endif()

	# Generate PDF from latex files
	if (DOC_GENERATE_PDF)
		add_custom_command(TARGET doc POST_BUILD
			COMMAND ${CMAKE_MAKE_PROGRAM}
			COMMAND ${CMAKE_COMMAND} -E make_directory ../pdf
			COMMAND ${CMAKE_COMMAND} -E copy refman.pdf ../pdf
			WORKING_DIRECTORY ${DOCS_DIRECTORY}/latex)
	endif()

	# Post-process man pages created by Doxygen
	if(DOC_GENERATE_MAN)
		add_custom_command(TARGET doc POST_BUILD
			COMMAND ${CMAKE_SOURCE_DIR}/auxbuild/buildman.sh
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${DOCS_DIRECTORY}/man/)
	endif()

	# Add documentation installation target (if the docs have been built)
	if (DOC_INSTALL_HTML)
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/html/
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/${PROJECT_NAME}/html OPTIONAL)
	endif()

	if (DOC_INSTALL_PDF)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/latex/refman.pdf
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/${PROJECT_NAME} OPTIONAL)
	endif()

	if (DOC_INSTALL_MAN)
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DOCS_DIRECTORY}/man/
			DESTINATION ${CMAKE_INSTALL_MANDIR} OPTIONAL
			FILES_MATCHING REGEX "(ccl|CCL).*gz")
	endif()

else()

	# No doc builds
	unset(DOC_BUILD CACHE)

	# Add doc target which warns that some document build dependencies
	# are missing
	add_custom_target(doc
		${CMAKE_COMMAND} -E echo "Documentation generation is disabled."
		COMMENT "One or more tools required for documentation generation were not found!"
		VERBATIM)

endif()

# ############### #
# PROCESS SUBDIRS #
# ############### #

# Add src folder
add_subdirectory(src)

# Add tests folder
enable_testing()
add_subdirectory(tests)

# Add scripts folder
add_subdirectory(scripts)

# ################# #
# PACKAGING SECTION #
# ################# #

# pkg-config support
configure_file(${CMAKE_SOURCE_DIR}/cf4ocl2.pc.in
	${CMAKE_BINARY_DIR}/generated/cf4ocl2.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/generated/cf4ocl2.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
	COMPONENT utilities)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)

# Determine architecture
target_architecture(BUILD_ARCH_GET)
set(PACKAGE_ARCH_NAME "${BUILD_ARCH_GET}" CACHE STRING "Build architecture (for packaging only)" FORCE)
mark_as_advanced(PACKAGE_ARCH_NAME)

# Generic packaging settings
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${${PROJECT_NAME}_VERSION_TWEAK}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A cross-platform pure C99 object-oriented framework for developing and benchmarking OpenCL projects in C/C++")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}.${PACKAGE_ARCH_NAME}")
set(CPACK_RESOURCE_VENDOR "fakenmc.com")

if(WIN32)
	# Windows packaging settings
	set(CPACK_GENERATOR "NSIS" "ZIP")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
	set(CPACK_NSIS_URL_INFO_ABOUT "http://fakenmc.github.io/cf4ocl/" )
elseif(APPLE)
	# OSX packaging settings
	# Use defaults
elseif(UNIX)
	# Linux packaging settings
	set(CPACK_GENERATOR "DEB" "RPM" "TGZ")

	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nuno Fachada <faken@fakenmc.com>")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0-0 (>= 2.32.1), ocl-icd-libopencl1 (>= 1.3)")
	set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ocl-icd-opencl-dev (>= 1.3), opencl-headers (>= 1.2)")

	set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
	set(CPACK_RPM_PACKAGE_REQUIRES "glib2 >= 2.32.1, ocl-icd >= 1.3")
	set(CPACK_RPM_PACKAGE_SUGGESTS "ocl-icd-devel >= 1.3, opencl-headers >= 1.2")

endif()

include (CPack)
