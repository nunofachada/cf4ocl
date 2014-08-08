# List of tests
set(TESTS test_error_handling test_cad_profiler test_cad_platforms 
	test_devquery test_cad_context test_cad_program test_op_profiler)
	
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
add_library(${PROJECT_NAME}_TESTING ../lib/errors.c ../lib/profiler.c 
	../lib/types.c ../lib/common.c ../lib/platforms.c
	../lib/kernel_arg.c ../lib/device_query.c ../lib/device_selector.c
	../lib/platform_wrapper.c ../lib/device_wrapper.c 
	../lib/context_wrapper.c ../lib/kernel_wrapper.c 
	../lib/program_wrapper.c ../lib/queue_wrapper.c 
	../lib/event_wrapper.c
	../lib/abstract_wrapper.c ../lib/abstract_dev_container_wrapper.c 
	../lib/memobj_wrapper.c ../lib/buffer_wrapper.c 
	../lib/image_wrapper.c)

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
set(test_op_profiler_LIBS ${PROJECT_NAME}_TESTING OpenCL_STUB_LIB)
set(test_cad_profiler_LIBS ${LIB_OPT})
set(test_cad_context_LIBS ${LIB_OPT})
set(test_cad_program_LIBS ${LIB_OPT})
set(test_cad_platforms_LIBS ${LIB_OPT})
set(test_devquery_LIBS ${LIB_OPT})

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${GLIB_LIBRARIES} ${${TEST}_LIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST} COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})
