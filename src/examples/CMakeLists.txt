# Build examples?
option(BUILD_EXAMPLES "Build examples?" ON)

# Stop processing if examples are not to be built
if (NOT ${BUILD_EXAMPLES})
	return()
endif()

# Examples without OpenCL kernel code
set(EXAMPLES_NOCL device_filter image_fill list_devices)

# Examples to be configured with OpenCL kernel code
set(EXAMPLES_CL image_filter ca canon)

# Specify location of stb headers for PNG load/save
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Configure examples using OpenCL kernel code
foreach(EXAMPLE ${EXAMPLES_CL})

	# Load kernel code into string
	#file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.cl ${EXAMPLE}_RAW)

	#string(REGEX REPLACE "(..)" "\\\\x\\1" ${EXAMPLE}_KERNEL_SRC
	#	${${EXAMPLE}_RAW})

	# Stringify
	execute_process(
		COMMAND "${CMAKE_SOURCE_DIR}/auxbuild/stringify.sh"
		INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.cl"
		OUTPUT_VARIABLE ${EXAMPLE}_KERNEL_SRC)

	# Configure example
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.c
		${CMAKE_BINARY_DIR}/examples/${EXAMPLE}.c @ONLY)

	# Add target for current example
	add_executable(${EXAMPLE} ${CMAKE_BINARY_DIR}/examples/${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE} ${PROJECT_NAME})

endforeach()

# Add a target for examples without OpenCL kernel code
foreach(EXAMPLE ${EXAMPLES_NOCL})
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
add_custom_target(examples DEPENDS ${EXAMPLES_NOCL} ${EXAMPLES_CL})
