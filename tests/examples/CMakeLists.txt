# Configure BATS file with locations of example binaries
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_examples.in.bats
    ${CMAKE_CURRENT_BINARY_DIR}/test_examples.bats @ONLY)

# Add automated ctest tests for utils, requires BATS
if (BATSEXEC)
    add_test(
        NAME test_examples
        COMMAND ${BATSEXEC} ${CMAKE_CURRENT_BINARY_DIR}/test_examples.bats)
endif()
