# Set tests log domain
remove_definitions(-DG_LOG_DOMAIN=\"${PROJECT_NAME}\")
add_definitions(-DG_LOG_DOMAIN=\"ccl-tests\")

# Remove disabling of assertions and checks for release builds
remove_definitions("-DG_DISABLE_ASSERT")
remove_definitions("-DG_DISABLE_CHECKS")

# Put test kernel in a string to insert into test.h
file(READ ${PROJECT_SOURCE_DIR}/tests/test_kernels/sum_full.cl
	TEST_KERNEL_SUM_RAW HEX)
string(REGEX REPLACE "(..)" "\\\\x\\1"
	TEST_KERNEL_SUM_SRC ${TEST_KERNEL_SUM_RAW})

# Configure include file for tests
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.in.h
	${CMAKE_BINARY_DIR}/generated/test.h @ONLY)

# Specify location of configured include file for tests
include_directories(${CMAKE_BINARY_DIR}/generated)

# Set of tests which only work with the OpenCL stub
set(TESTS_STUBONLY test_profiler_op)

# Set of tests which work with either the OpenCL stub or a real OpenCL
# implementation
set(TESTS_OPT test_profiler test_platforms test_buffer test_devquery
	test_context test_event test_program test_image test_sampler
	test_kernel test_queue test_device test_devsel)

# Complete set of tests
set(TESTS ${TESTS_STUBONLY} ${TESTS_OPT})

# Add subdir to build OpenCL stub lib
add_subdirectory(ocl_stub)

# Static cf4ocl library for tests, will use the OpenCL stub
set(CCL_LIB_DIR "${CMAKE_SOURCE_DIR}/src/lib/ccl_")
add_library(${PROJECT_NAME}_TESTING STATIC
	${CCL_LIB_DIR}errors.c ${CCL_LIB_DIR}profiler.c
	${CCL_LIB_DIR}common.c
	${CCL_LIB_DIR}platforms.c ${CCL_LIB_DIR}kernel_arg.c
	${CCL_LIB_DIR}device_query.c ${CCL_LIB_DIR}device_selector.c
	${CCL_LIB_DIR}platform_wrapper.c ${CCL_LIB_DIR}device_wrapper.c
	${CCL_LIB_DIR}context_wrapper.c ${CCL_LIB_DIR}kernel_wrapper.c
	${CCL_LIB_DIR}program_wrapper.c ${CCL_LIB_DIR}queue_wrapper.c
	${CCL_LIB_DIR}event_wrapper.c ${CCL_LIB_DIR}abstract_wrapper.c
	${CCL_LIB_DIR}abstract_dev_container_wrapper.c
	${CCL_LIB_DIR}memobj_wrapper.c ${CCL_LIB_DIR}buffer_wrapper.c
	${CCL_LIB_DIR}image_wrapper.c ${CCL_LIB_DIR}sampler_wrapper.c)

set_target_properties(${PROJECT_NAME}_TESTING PROPERTIES
	COMPILE_FLAGS "-DCCL_STATIC_DEFINE")

# Dependencies for the static cf4ocl library for tests
target_link_libraries(${PROJECT_NAME}_TESTING ${GLIB_LDFLAGS} OpenCL_STUB_LIB)

# Use OpenCL stub when possible?
option(TESTS_USE_OPENCL_STUB "Use OpenCL stub in tests when possible?" ON)

# Determine general properties for tests which can either use the OpenCL
# stub or a real OpenCL implementation
if (TESTS_USE_OPENCL_STUB)
	set(LIB_OPT ${PROJECT_NAME}_TESTING)
	add_definitions("-DOPENCL_STUB")
	set(FLAGS_OPT "-DCCL_STATIC_DEFINE")
else()
	set(LIB_OPT ${PROJECT_NAME})
	set(FLAGS_OPT "")
endif()

# Set properties for tests which can either use the OpenCL stub
# or a real OpenCL implementation
foreach(TEST_OPT ${TESTS_OPT})
	set(${TEST_OPT}_LIBS ${LIB_OPT})
	set(${TEST_OPT}_FLAGS "${FLAGS_OPT}")
endforeach()

# Set properties for tests which can only use the OpenCL stub
foreach(TEST_STUBONLY ${TESTS_STUBONLY})
	set(${TEST_STUBONLY}_LIBS ${PROJECT_NAME}_TESTING)
	set(${TEST_STUBONLY}_FLAGS "-DCCL_STATIC_DEFINE")
endforeach()

# Add a target for each test
foreach(TEST ${TESTS})
	add_executable(${TEST} ${TEST}.c test.c)
	target_link_libraries(${TEST} ${${TEST}_LIBS})
	set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST}
		COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR} ${${TEST}_FLAGS}")
endforeach(TEST)

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})
