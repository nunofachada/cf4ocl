# Build examples?
option(BUILD_EXAMPLES "Build and install examples?" ON)

# Library, examples and utilities are included in the ALL target
add_subdirectory(lib)
add_subdirectory(utils)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# Tests are not included in the ALL target
add_subdirectory(tests EXCLUDE_FROM_ALL)

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION include)
