# Library, examples and utilities are included in the ALL target
add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(utils)
add_subdirectory(tests)

# Where to copy aggregate header for compilation purposes
file(COPY ${PROJECT_NAME}.h
	DESTINATION ${CMAKE_BINARY_DIR}/include)

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

