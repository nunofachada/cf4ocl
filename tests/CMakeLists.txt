# Build tests?
option(BUILD_TESTS "Build tests?" ON)

# Stop processing if tests are not to be built
if (NOT ${BUILD_TESTS})
    unset(TESTS_USE_OPENCL_STUB CACHE)
    unset(TESTS_DEVICE_INDEX CACHE)
    unset(TESTS_GENERATE_COVERAGE CACHE)
    return()
endif()

# Try to find BATS for testing utilities and examples
find_program(BATSEXEC bats)
mark_as_advanced(BATSEXEC)

# Allow the user to specify which device to use in generic device tests
set(TESTS_DEVICE_INDEX "0" CACHE STRING
    "Index of device to use in generic device tests")

# Add test folders
add_subdirectory(examples)
add_subdirectory(lib)
add_subdirectory(utils)

# Configure helper script to perform all tests in all available devices
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_all.in.sh
    ${CMAKE_CURRENT_BINARY_DIR}/test_all.sh @ONLY)

# Code coverage configuration
if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))

    # Option for generating code coverage reports
    option(TESTS_GENERATE_COVERAGE "Generate code coverage?" OFF)

    # If option for generating code coverage reports is on...
    if (TESTS_GENERATE_COVERAGE)

        # Include code coverage settings
        include(CodeCoverage)

        # Code coverage program locations should be marked as advanced
        mark_as_advanced(GCOVR_PATH GCOV_PATH GENHTML_PATH LCOV_PATH)

        # Append coverage compiler flags
        append_coverage_compiler_flags()

        # Define exclude folders
        set(COVERAGE_LCOV_EXCLUDES '/usr/*')

        # ...setup test target for coverage
        setup_target_for_coverage_lcov(
            NAME codecov                       # Name for custom target
            EXECUTABLE ${CMAKE_CTEST_COMMAND}  # Name of the test driver executable that runs the tests
            DEPENDENCIES test                  # Dependencies to build first
        )

    endif()

else()

    # Unset code coverage generation options
    unset(TESTS_GENERATE_COVERAGE CACHE)

endif()
