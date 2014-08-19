# Sources required for building library
set(SRC errors.c profiler.c types.c common.c platforms.c
	kernel_arg.c device_query.c device_selector.c
	platform_wrapper.c device_wrapper.c context_wrapper.c 
	kernel_wrapper.c program_wrapper.c queue_wrapper.c event_wrapper.c
	abstract_wrapper.c abstract_dev_container_wrapper.c 
	memobj_wrapper.c buffer_wrapper.c image_wrapper.c sampler_wrapper.c)

# Setup the configuration header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common.in.h 
	${CMAKE_BINARY_DIR}/cf4ocl2/common.h @ONLY)

# Add library
add_library(${PROJECT_NAME} SHARED ${SRC})

# This is required for Windows compilation
target_link_libraries(${PROJECT_NAME} 
	${GLIB_LIBRARIES} ${GLIB_LDFLAGS} ${OPENCL_LIBRARIES})

# This target is just an alias for cf4ocl
add_custom_target(lib DEPENDS ${PROJECT_NAME})

# Set library version
set_target_properties(${PROJECT_NAME} PROPERTIES 
	VERSION ${${PROJECT_NAME}_VERSION_STRING}
	SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

# Where to install lib
install(TARGETS ${PROJECT_NAME} 
	LIBRARY DESTINATION ${INSTALL_SUBDIR_LIB}
	ARCHIVE DESTINATION ${INSTALL_SUBDIR_LIB}
	RUNTIME DESTINATION ${INSTALL_SUBDIR_BIN})

# Where to install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${INSTALL_SUBDIR_INCLUDE}/${PROJECT_NAME}
	FILES_MATCHING PATTERN "*.h"
	PATTERN "*.in.h" EXCLUDE)

# Install the common header
install(FILES ${CMAKE_BINARY_DIR}/common.h
         DESTINATION ${INSTALL_SUBDIR_INCLUDE}/${PROJECT_NAME})
