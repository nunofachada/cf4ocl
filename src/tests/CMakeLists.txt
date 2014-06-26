# List of tests
set(TESTS test_gerrorf test_profiler test_wrappers test_devquery)

# Use OpenCL stub when possible?
option(USE_OPENCL_STUB "Use OpenCL stub when possible?" ON)

# Common libraries for tests
set(COMMON_LIBS ${GLIB_LIBRARIES})

# CF4OCL library for tests
set(CF4OCL_SRC ../lib/platforms.c ../lib/platform.c ../lib/device.c 
	../lib/errors.c ../lib/common.c ../lib/devquery.c ../lib/devsel.c 
	../lib/context.c ../lib/wrapper.c ../lib/profiler.c ../lib/cqueue.c
	../lib/event.c)

# Specific libraries for tests
add_library(test_profiler_lib ${CF4OCL_SRC} mocks_stubs/profiler/cl_stub.c)
set_target_properties(test_profiler_lib PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")
set(test_profiler_EXTRALIBS test_profiler_lib)

if (USE_OPENCL_STUB)

	add_library(test_lib ${CF4OCL_SRC} mocks_stubs/wrappers/cl_stub.c)
	set(test_wrappers_EXTRALIBS test_lib)
	set(test_devquery_EXTRALIBS test_lib)

else()

	add_library(test_lib ${CF4OCL_SRC})
	set(test_wrappers_EXTRALIBS test_lib ${OPENCL_LIBRARIES})
	set(test_devquery_EXTRALIBS test_lib ${OPENCL_LIBRARIES})

endif()
set_target_properties(test_lib PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${COMMON_LIBS} ${${TEST}_EXTRALIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST} COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})



