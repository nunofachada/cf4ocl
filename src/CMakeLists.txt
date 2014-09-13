# Build examples?
option(BUILD_EXAMPLES "Build examples?" ON)

# Build utils?
option(BUILD_UTILS "Build and install utilities?" ON)

# Build tests?
option(BUILD_TESTS "Build tests?" ON)

# Library, examples and utilities are included in the ALL target
add_subdirectory(cf4ocl2)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if (BUILD_UTILS)
	add_subdirectory(utils)
endif()
if (BUILD_TESTS)
	add_subdirectory(tests)
endif()

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION ${INSTALL_SUBDIR_INCLUDE})

