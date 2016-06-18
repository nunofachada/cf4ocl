# Build examples?
option(BUILD_EXAMPLES "Build examples?" ON)

# Stop processing if examples are not to be built
if (NOT ${BUILD_EXAMPLES})
	return()
endif()

# List of examples
SET(EXAMPLES
	image_fill
	image_filter
	list_devices
	ca
	device_filter
	canon)

# List of headers to be configured with OpenCL source code
SET(HEADERS image_filter ca canon)

# Configure headers
foreach(HEADER ${HEADERS})

	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}.cl ${HEADER}_RAW HEX)
	string(REGEX REPLACE "(..)" "\\\\x\\1" ${HEADER}_KERNEL_SRC
		${${HEADER}_RAW})

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}.in.h
		${CMAKE_BINARY_DIR}/generated/${HEADER}.h @ONLY)

endforeach()

# Specify location of headers
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}    # Non-modified headers
	${CMAKE_BINARY_DIR}/generated) # Configured headers

# Add a target for each example
foreach(EXAMPLE ${EXAMPLES})
	add_executable(${EXAMPLE} ${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE} ${PROJECT_NAME})
endforeach()

# The stb_image.h included in image_filter throws some warnings,
# this avoids them.
if((${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	OR (${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))

	set_target_properties(image_filter PROPERTIES
		COMPILE_FLAGS "-Wno-unused-parameter -Wno-sign-compare")

endif()

# Add a target which builds all samples
add_custom_target(examples DEPENDS ${EXAMPLES})
