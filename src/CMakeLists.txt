# Library, examples and utilities are included in the ALL target
add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(utils)

# Tests are not included in the ALL target
add_subdirectory(tests EXCLUDE_FROM_ALL)

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION include)
