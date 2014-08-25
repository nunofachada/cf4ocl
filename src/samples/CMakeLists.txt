# List of examples
SET(SAMPLES image_fill)

# Get helper image lib
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/stb_image_write.h)
	file(DOWNLOAD https://github.com/nothings/stb/raw/master/stb_image_write.h
		${CMAKE_CURRENT_LIST_DIR}/stb_image_write.h
		STATUS stb_status_lst INACTIVITY_TIMEOUT 10)
	list(GET stb_status_lst 0 stb_status)
	if (${stb_status})
		message(FATAL_ERROR "Unable to download stb_image_write.h")
	endif()
endif()

# Add a target for each example
foreach(SAMPLE ${SAMPLES})
	add_executable(${SAMPLE} ${SAMPLE}.c)
	target_link_libraries(${SAMPLE} ${PROJECT_NAME} ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})
endforeach()

# Add a target which builds all samples
add_custom_target(samples DEPENDS ${SAMPLES})
