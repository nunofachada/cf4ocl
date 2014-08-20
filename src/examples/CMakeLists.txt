# List of examples
SET(EXAMPLES matmult bank_conflicts)

# Common libraries for examples
set(COMMON_LIBS ${PROJECT_NAME} ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES} 
	examples_common)
set(matmult_EXTRALIBS m)

# Use OpenMP in matmult example?
find_package(OpenMP QUIET)

# Add common examples library
add_library(examples_common examples_common.c)

# Add a target for each example
foreach(EXAMPLE ${EXAMPLES})
	add_executable(${EXAMPLE} ${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE} ${COMMON_LIBS} ${${EXAMPLE}_EXTRALIBS})
	# Copy the OpenCL kernel to the same location as the example executable
	add_custom_command(TARGET ${EXAMPLE} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.cl
		$<TARGET_FILE_DIR:${EXAMPLE}>
	)
endforeach(EXAMPLE)

# Set specific matmult properties
if (OPENMP_FOUND)
	set_target_properties(matmult PROPERTIES 
		COMPILE_FLAGS "${OpenMP_C_FLAGS} -DCF4OCL_USE_OPENMP" 
		LINK_FLAGS ${OpenMP_C_FLAGS})
endif()

# Add a target which builds all examples
add_custom_target(examples DEPENDS ${EXAMPLES})
