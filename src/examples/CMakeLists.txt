# List of examples
SET(EXAMPLES image_fill image_filter list_devices cellular_automata
	device_filter canon)

# Add a target for each example
foreach(EXAMPLE ${EXAMPLES})
	add_executable(${EXAMPLE} ${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE}
		${PROJECT_NAME} ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})
endforeach()

# The stb_image.h included in image_filter throws some warnings,
# this avoids them.
set_target_properties(image_filter PROPERTIES
	COMPILE_FLAGS "-Wno-unused-parameter -Wno-sign-compare")

# Add a target which builds all samples
add_custom_target(examples DEPENDS ${EXAMPLES})
