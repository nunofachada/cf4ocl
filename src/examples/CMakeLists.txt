set(COMMON_LIBS cf4ocl ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES} exp_common)
find_package(OpenMP REQUIRED)

add_library(exp_common exp_common.c)

add_executable(cf4ocl_matmult matmult.c)
target_link_libraries(cf4ocl_matmult ${COMMON_LIBS} m)
set_target_properties(cf4ocl_matmult PROPERTIES COMPILE_FLAGS ${OpenMP_C_FLAGS} LINK_FLAGS ${OpenMP_C_FLAGS})

add_executable(cf4ocl_bank_conflicts bank_conflicts.c)
target_link_libraries(cf4ocl_bank_conflicts ${COMMON_LIBS})

