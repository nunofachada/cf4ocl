# Sources required for building library
set(SRC ccl_errors.c ccl_profiler.c ccl_common.c ccl_platforms.c
	ccl_kernel_arg.c ccl_device_query.c ccl_device_selector.c
	ccl_platform_wrapper.c ccl_device_wrapper.c ccl_context_wrapper.c
	ccl_kernel_wrapper.c ccl_program_wrapper.c ccl_queue_wrapper.c
	ccl_event_wrapper.c ccl_abstract_wrapper.c
	ccl_abstract_dev_container_wrapper.c ccl_memobj_wrapper.c
	ccl_buffer_wrapper.c ccl_image_wrapper.c ccl_sampler_wrapper.c)

# Setup the configuration header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ccl_common.in.h
	${CMAKE_BINARY_DIR}/cf4ocl2/ccl_common.h @ONLY)

# Add library
add_library(${PROJECT_NAME} SHARED ${SRC})
generate_export_header(${PROJECT_NAME} BASE_NAME ccl)

# This is required for Windows compilation
target_link_libraries(${PROJECT_NAME}
	${GLIB_LIBRARIES} ${GLIB_LDFLAGS} ${OPENCL_LIBRARIES})

# This target is just an alias for cf4ocl
add_custom_target(lib DEPENDS ${PROJECT_NAME})

# Set library version
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION}
	SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

# Where to install lib
install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Where to install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
	FILES_MATCHING PATTERN "ccl_*.h"
	PATTERN "*.in.h" EXCLUDE)

# Install the common header and the export header
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/ccl_common.h
	${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}/ccl_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
