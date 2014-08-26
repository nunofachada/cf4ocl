# List of examples
SET(SAMPLES image_fill image_filter)

# Add a target for each example
foreach(SAMPLE ${SAMPLES})
	add_executable(${SAMPLE} ${SAMPLE}.c)
	target_link_libraries(${SAMPLE}
		${PROJECT_NAME} ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})
endforeach()

# The stb_image.h included in image_filter throws some warnings,
# this avoids them.
set_target_properties(image_filter PROPERTIES
	COMPILE_FLAGS "-Wno-unused-parameter -Wno-sign-compare")

# Add a target which builds all samples
add_custom_target(samples DEPENDS ${SAMPLES})
