# Build tests?
option(BUILD_TESTS "Build tests?" ON)

# Stop processing if tests are not to be built
if (NOT ${BUILD_TESTS})
	unset(DOC_EXCLUDE_TESTS CACHE)
	unset(TESTS_USE_OPENCL_STUB CACHE)
	unset(TESTS_DEVICE_INDEX CACHE)
	return()
endif()

# Allow the user to specify which device to use in generic device tests
set(TESTS_DEVICE_INDEX "0" CACHE STRING
	"Index of device to use in generic device tests")

# Add lib and utils test folders
add_subdirectory(lib)
add_subdirectory(utils)
