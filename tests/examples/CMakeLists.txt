# Configure BATS file with locations of example binaries
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_examples.in.bats
	${CMAKE_CURRENT_BINARY_DIR}/test_examples.bats @ONLY)
