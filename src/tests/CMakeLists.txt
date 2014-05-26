# List of tests
set(TESTS test_gerrorf test_profiler)

# Common and specific libraries for tests
set(COMMON_LIBS ${GLIB_LIBRARIES})
set(test_profiler_EXTRALIBS clprofiler)

# Add clprofiler library for testing
add_library(clprofiler ../lib/profiler.c ../lib/errors.c)
set_target_properties(clprofiler PROPERTIES COMPILE_FLAGS "-DTESTING -I${CMAKE_CURRENT_LIST_DIR}")

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c)
	target_link_libraries(${TEST} ${COMMON_LIBS} ${${TEST}_EXTRALIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${EXECS_PREFIX}${TEST} COMPILE_FLAGS "-DTESTING -I${CMAKE_CURRENT_LIST_DIR}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})



