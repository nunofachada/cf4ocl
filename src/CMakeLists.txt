# Library, examples and utilities are included in the ALL target
add_subdirectory(cf4ocl2)
add_subdirectory(examples)
add_subdirectory(utils)
add_subdirectory(tests)

# Where to install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION ${INSTALL_SUBDIR_INCLUDE})

