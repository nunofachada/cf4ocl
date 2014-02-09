# Library, examples and utilities are included in the ALL target
add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(utils)

# Tests are not included in the ALL target
add_subdirectory(tests EXCLUDE_FROM_ALL)
