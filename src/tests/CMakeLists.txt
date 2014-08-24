# List of tests
set(TESTS test_error_handling test_profiler test_platforms test_buffer
	test_devquery test_context test_program test_profiler_op test_image
	test_sampler)
	
# Exclude tests from documentation?
if (DOXYGEN_FOUND) 
	option(DOC_EXCLUDE_TESTS "Exclude tests from API docs?" ON)
	if (DOC_EXCLUDE_TESTS)
		# Add tests path to doc exclude path
		set(DOC_EXCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR} ${DOC_EXCLUDE_PATHS}" CACHE INTERNAL "")
	endif()
endif()

# Add subdir to build OpenCL stub lib
add_subdirectory(ocl_stub)

# CF4OCL library for tests
set(CCL_LIB_DIR "../cf4ocl2/")
add_library(${PROJECT_NAME}_TESTING 
	${CCL_LIB_DIR}errors.c ${CCL_LIB_DIR}profiler.c 
	${CCL_LIB_DIR}types.c ${CCL_LIB_DIR}common.c 
	${CCL_LIB_DIR}platforms.c ${CCL_LIB_DIR}kernel_arg.c
	${CCL_LIB_DIR}device_query.c ${CCL_LIB_DIR}device_selector.c
	${CCL_LIB_DIR}platform_wrapper.c ${CCL_LIB_DIR}device_wrapper.c 
	${CCL_LIB_DIR}context_wrapper.c ${CCL_LIB_DIR}kernel_wrapper.c 
	${CCL_LIB_DIR}program_wrapper.c ${CCL_LIB_DIR}queue_wrapper.c 
	${CCL_LIB_DIR}event_wrapper.c ${CCL_LIB_DIR}abstract_wrapper.c 
	${CCL_LIB_DIR}abstract_dev_container_wrapper.c 
	${CCL_LIB_DIR}memobj_wrapper.c ${CCL_LIB_DIR}buffer_wrapper.c 
	${CCL_LIB_DIR}image_wrapper.c ${CCL_LIB_DIR}sampler_wrapper.c)

target_link_libraries(${PROJECT_NAME}_TESTING 
	${GLIB_LIBRARIES} ${GLIB_LDFLAGS} OpenCL_STUB_LIB)

# Use OpenCL stub when possible?
option(USE_OPENCL_STUB "Use OpenCL stub when possible?" ON)

if (USE_OPENCL_STUB)
	set(LIB_OPT ${PROJECT_NAME}_TESTING OpenCL_STUB_LIB)
	add_definitions(-DOPENCL_STUB)
else()
	set(LIB_OPT ${PROJECT_NAME} ${OPENCL_LIBRARIES})
endif()

# Specific specific libraries for tests
set(test_profiler_op_LIBS ${PROJECT_NAME}_TESTING OpenCL_STUB_LIB)
set(test_profiler_LIBS ${LIB_OPT})
set(test_context_LIBS ${LIB_OPT})
set(test_program_LIBS ${LIB_OPT})
set(test_platforms_LIBS ${LIB_OPT})
set(test_devquery_LIBS ${LIB_OPT})
set(test_buffer_LIBS ${LIB_OPT}) #${PROJECT_NAME} ${OPENCL_LIBRARIES}
set(test_image_LIBS ${PROJECT_NAME} ${OPENCL_LIBRARIES})
set(test_sampler_LIBS ${PROJECT_NAME} ${OPENCL_LIBRARIES})
set(test_error_handling_LIBS ${PROJECT_NAME}_TESTING)

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${GLIB_LIBRARIES} ${${TEST}_LIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST} COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})
