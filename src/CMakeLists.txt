# Build examples?
option(BUILD_EXAMPLES "Build and install examples?" ON)

# Build utils?
option(BUILD_UTILS "Build and install utilities?" ON)

# Library, examples and utilities are included in the ALL target
add_subdirectory(lib)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if (BUILD_UTILS)
	add_subdirectory(utils)
endif()

# Tests are not included in the ALL target
add_subdirectory(tests EXCLUDE_FROM_ALL)

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION ${INSTALL_SUBDIR_INCLUDE})

