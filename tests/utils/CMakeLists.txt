# Copy tests to build folder
file(COPY .
	DESTINATION ${CMAKE_BINARY_DIR}/tests/utils
	FILES_MATCHING PATTERN "*.bats")
