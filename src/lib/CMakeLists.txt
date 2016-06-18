# Sources required for building library
set(SRC ccl_errors.c ccl_profiler.c ccl_common.c ccl_platforms.c
	ccl_kernel_arg.c ccl_device_query.c ccl_device_selector.c
	ccl_platform_wrapper.c ccl_device_wrapper.c ccl_context_wrapper.c
	ccl_kernel_wrapper.c ccl_program_wrapper.c ccl_queue_wrapper.c
	ccl_event_wrapper.c ccl_abstract_wrapper.c
	ccl_abstract_dev_container_wrapper.c ccl_memobj_wrapper.c
	ccl_buffer_wrapper.c ccl_image_wrapper.c ccl_sampler_wrapper.c)

# Special debug mode for logging lifetime (new/destroy) of wrapper objects
if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	option(DEBUG_OBJ_LIFETIME
		"Log debug messages for wrapper object new/destroy?" OFF)
else()
	unset(DEBUG_OBJ_LIFETIME CACHE)
endif()

if (DEBUG_OBJ_LIFETIME)
	add_definitions("-DCCL_DEBUG_OBJ_LIFETIME")
else()
	remove_definitions("-DCCL_DEBUG_OBJ_LIFETIME")
endif()

# Setup the configuration header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ccl_common.in.h
	${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}/ccl_common.h @ONLY)

# Copy aggregate header for compilation purposes
file(COPY ${PROJECT_NAME}.h
	DESTINATION ${CMAKE_BINARY_DIR}/include)

# Copy headers for compilation purposes
file(COPY .
	DESTINATION ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}
	FILES_MATCHING PATTERN "*.h"
	PATTERN "*.in.h" EXCLUDE
	PATTERN "${PROJECT_NAME}.h" EXCLUDE)

# Add library
add_library(${PROJECT_NAME} SHARED ${SRC})

# Generate export header and copy it to the includes folder
generate_export_header(${PROJECT_NAME} BASE_NAME ccl)
file(COPY ${CMAKE_BINARY_DIR}/src/lib/ccl_export.h
	DESTINATION ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Specify dependencies
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${OPENCL_LIBRARIES})

# This target is just an alias for cf4ocl
add_custom_target(lib DEPENDS ${PROJECT_NAME})

# Set library version
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION}
	SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

# Install library
install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install aggregate header
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install headers
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "ccl_*.h")
