# List of tests
set(TESTS test_gerrorf test_profiler test_wrappers)

# Common libraries for tests
set(COMMON_LIBS ${GLIB_LIBRARIES})

# Specific libraries for tests
add_library(test_profiler_lib ../lib/profiler.c ../lib/errors.c ../lib/common.c mocks_stubs/profiler/cl_stub.c)
set_target_properties(test_profiler_lib PROPERTIES COMPILE_FLAGS "-DTESTING -I${CMAKE_CURRENT_LIST_DIR}")
set(test_profiler_EXTRALIBS test_profiler_lib)

add_library(test_wrappers_lib ../lib/platforms.c ../lib/platform.c ../lib/device.c ../lib/errors.c ../lib/common.c)
set_target_properties(test_wrappers_lib PROPERTIES COMPILE_FLAGS "-DTESTING -I${CMAKE_CURRENT_LIST_DIR}")
set(test_wrappers_EXTRALIBS test_wrappers_lib ${OPENCL_LIBRARIES})

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${COMMON_LIBS} ${${TEST}_EXTRALIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST} COMPILE_FLAGS "-DTESTING -I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})



