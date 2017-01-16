# Configure BATS file with locations of executables
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_c.in.bats
	${CMAKE_CURRENT_BINARY_DIR}/test_c.bats @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_devinfo.in.bats
	${CMAKE_CURRENT_BINARY_DIR}/test_devinfo.bats @ONLY)

# Add automated ctest tests for utils, requires BATS
if (BATSEXEC)
	add_test(
		NAME test_util_c
		COMMAND ${BATSEXEC} ${CMAKE_CURRENT_BINARY_DIR}/test_c.bats)
	add_test(
		NAME test_util_devinfo
		COMMAND ${BATSEXEC} ${CMAKE_CURRENT_BINARY_DIR}/test_devinfo.bats)
endif()
