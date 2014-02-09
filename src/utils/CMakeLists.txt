set(COMMON_LIBS cf4ocl ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})

add_executable(cf4ocl_device_query device_query.c)
target_link_libraries(cf4ocl_device_query ${COMMON_LIBS})

add_executable(cf4ocl_kernel_info kernel_info.c)
target_link_libraries(cf4ocl_kernel_info ${COMMON_LIBS})


