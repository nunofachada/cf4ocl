# Build tests?
option(BUILD_TESTS "Build tests?" ON)

# Stop processing if tests are not to be built
if (NOT ${BUILD_TESTS})
	unset(DOC_EXCLUDE_TESTS CACHE)
	unset(TESTS_USE_OPENCL_STUB CACHE)
	unset(TESTS_DEVICE_INDEX CACHE)
	unset(TESTS_COVERALLS CACHE)
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
