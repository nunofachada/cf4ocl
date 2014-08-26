# List of examples
SET(SAMPLES image_fill)

# Add a target for each example
foreach(SAMPLE ${SAMPLES})
	add_executable(${SAMPLE} ${SAMPLE}.c)
	target_link_libraries(${SAMPLE}
		${PROJECT_NAME} ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})
endforeach()

# Add a target which builds all samples
add_custom_target(samples DEPENDS ${SAMPLES})
