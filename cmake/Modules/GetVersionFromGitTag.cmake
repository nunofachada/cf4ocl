# 
# This cmake module sets the project version and partial version 
# variables by analysing the git tag and commit history. It expects git 
# tags defined with semantic versioning 2.0.0 (http://semver.org/).
#
# The module expects the PROJECT_NAME and GIT_EXECUTABLE variables to be
# set, and once done will define the following variables:
#
# ${PROJECT_NAME}_VERSION_STRING - Version string without metadata
# such as "v2.0.0" or "v.1.2.41-beta.1". This should correspond to the
# most recent git tag.
# ${PROJECT_NAME}_VERSION_STRING_FULL - Version string with metadata
# such as "v2.0.0+3.a23fbc" or "v1.3.1-alpha.2+4.9c4fd1"
# ${PROJECT_NAME}_VERSION_MAJOR - Major version integer (e.g. 2 in v2.3.1-RC.2+21.ef12c8)
# ${PROJECT_NAME}_VERSION_MINOR - Minor version integer (e.g. 3 in v2.3.1-RC.2+21.ef12c8)
# ${PROJECT_NAME}_VERSION_PATCH - Patch version integer (e.g. 1 in v2.3.1-RC.2+21.ef12c8)
# ${PROJECT_NAME}_VERSION_TWEAK - Tweak version string (e.g. "RC-2" in v2.3.1-RC.2+21.ef12c8)
# ${PROJECT_NAME}_VERSION_AHEAD - How many commits ahead of last tag (e.g. 21 in v2.3.1-RC.2+21.ef12c8)
# ${PROJECT_NAME}_VERSION_GIT_SHA - The git sha1 of the most recent commit (e.g. the "ef12c8" in v2.3.1-RC.2+21.ef12c8)
#
# This module is public domain, use it as it fits you best.
#
# Author: Nuno Fachada

# Don't use this module without calling FindGit first
if (NOT ${GIT_FOUND})
	message(FATAL_ERROR "This script requires Git")
endif()

# Get last tag from git
execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_STRING
	OUTPUT_STRIP_TRAILING_WHITESPACE)

#How many commits since last tag
execute_process(COMMAND ${GIT_EXECUTABLE} rev-list master ${${PROJECT_NAME}_VERSION_STRING}^..HEAD --count
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_AHEAD
	OUTPUT_STRIP_TRAILING_WHITESPACE)	

# Get current commit SHA from git
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_GIT_SHA 
	OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get partial versions into a list
string(REGEX MATCHALL "-.*$|[0-9]+" ${PROJECT_NAME}_PARTIAL_VERSION_LIST
	${${PROJECT_NAME}_VERSION_STRING})

# Set the version numbers
list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST 
	0 ${PROJECT_NAME}_VERSION_MAJOR)
list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST 
	1 ${PROJECT_NAME}_VERSION_MINOR)
list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST 
	2 ${PROJECT_NAME}_VERSION_PATCH)

# The tweak part is optional, so check if the list contains it
list(LENGTH ${PROJECT_NAME}_PARTIAL_VERSION_LIST 
	${PROJECT_NAME}_PARTIAL_VERSION_LIST_LEN)
if (${PROJECT_NAME}_PARTIAL_VERSION_LIST_LEN GREATER 3)
	list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST 3 ${PROJECT_NAME}_VERSION_TWEAK)
	string(SUBSTRING ${${PROJECT_NAME}_VERSION_TWEAK} 1 -1 ${PROJECT_NAME}_VERSION_TWEAK)
endif()

# Unset the list
unset(${PROJECT_NAME}_PARTIAL_VERSION_LIST)

# Set full project version string
set(${PROJECT_NAME}_VERSION_STRING_FULL 
	${${PROJECT_NAME}_VERSION_STRING}+${${PROJECT_NAME}_VERSION_AHEAD}.${${PROJECT_NAME}_VERSION_GIT_SHA})
