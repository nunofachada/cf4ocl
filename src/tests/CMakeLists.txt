# List of tests
set(TESTS test_gerrorf test_profiler test_wrappers test_devquery)

# Add subdir to build OpenCL stub lib
add_subdirectory(ocl_stub)

# CF4OCL library for tests
add_library(${PROJECT_NAME}_TESTING ../lib/platforms.c ../lib/platform.c 
	../lib/device.c ../lib/errors.c ../lib/common.c ../lib/devquery.c 
	../lib/devsel.c ../lib/context.c ../lib/wrapper.c ../lib/profiler.c 
	../lib/cqueue.c ../lib/event.c ../lib/kernel.c ../lib/program.c
	../lib/abstract_dev_container_wrapper.c ../lib/arg.c ../lib/memobj.c
	../lib/buffer.c ../lib/image.c)

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
set(test_profiler_LIBS ${PROJECT_NAME}_TESTING OpenCL_STUB_LIB)
set(test_wrappers_LIBS ${LIB_OPT})
set(test_devquery_LIBS ${LIB_OPT})

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${GLIB_LIBRARIES} ${${TEST}_LIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST} COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})
