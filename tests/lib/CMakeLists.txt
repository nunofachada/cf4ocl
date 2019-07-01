# Set tests log domain
remove_definitions(-DG_LOG_DOMAIN=\"${PROJECT_NAME}\")
add_definitions(-DG_LOG_DOMAIN=\"ccl-tests\")

# Remove disabling of assertions and checks for release builds
remove_definitions("-DG_DISABLE_ASSERT")
remove_definitions("-DG_DISABLE_CHECKS")

# Library code files to be tested
file(GLOB CODE2TEST "${CMAKE_SOURCE_DIR}/src/lib/*.c")

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

# Set of tests to build
set(TESTS test_profiler test_platforms test_buffer test_devquery test_context
    test_event test_program test_image test_sampler test_kernel test_queue
    test_device test_devsel test_errors)

# Add a target for each test
foreach(TEST ${TESTS})
    add_executable(${TEST} ${TEST}.c test.c)
    target_link_libraries(${TEST} ${PROJECT_NAME})
    set_target_properties(${TEST} PROPERTIES OUTPUT_NAME ${TEST}
        COMPILE_FLAGS "-I${CMAKE_CURRENT_LIST_DIR} ${${TEST}_FLAGS}")
    # Add test to ctest
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()

# Add a target which builds all tests
add_custom_target(tests DEPENDS ${TESTS})
