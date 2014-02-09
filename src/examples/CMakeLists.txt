# List of examples
SET(EXAMPLES matmult bank_conflicts)

# Common libraries for examples
set(COMMON_LIBS cf4ocl ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES} exp_common)
set(matmult_EXTRALIBS m)

# OpenMP required by matmult example
find_package(OpenMP REQUIRED)

# Add common examples library
add_library(exp_common exp_common.c)

# Add a target for each example
foreach(EXAMPLE ${EXAMPLES})
	add_executable(${EXAMPLE} ${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE} ${COMMON_LIBS} ${${EXAMPLE}_EXTRALIBS})
	set_target_properties(${EXAMPLE} PROPERTIES OUTPUT_NAME ${EXECS_PREFIX}${EXAMPLE})
endforeach(EXAMPLE)

# Set specific matmult properties
set_target_properties(matmult PROPERTIES COMPILE_FLAGS ${OpenMP_C_FLAGS} LINK_FLAGS ${OpenMP_C_FLAGS})

# Add a target which builds all examples
add_custom_target(examples DEPENDS ${EXAMPLES})

