# Configure BATS file with locations of executables
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_c.in.bats
	${CMAKE_CURRENT_BINARY_DIR}/test_c.bats @ONLY)
