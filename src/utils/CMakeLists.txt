# List of utilities
set(UTILS device_query kernel_info)

# Common libs for utilities
set(COMMON_LIBS cf4ocl ${OPENCL_LIBRARIES} ${GLIB_LIBRARIES})

# Add a target for each utility
foreach(UTIL ${UTILS})
	add_executable(${UTIL} ${UTIL}.c)
	target_link_libraries(${UTIL} ${COMMON_LIBS})
	set_target_properties(${UTIL} PROPERTIES OUTPUT_NAME ${EXECS_PREFIX}${UTIL})
endforeach(UTIL)

# Set utils version
set_target_properties(${UTILS} PROPERTIES 
	VERSION 
		${cf4ocl_VERSION_MAJOR}.${cf4ocl_VERSION_MINOR}.${cf4ocl_VERSION_PATCH})

# How to install utils
install(TARGETS ${UTILS} RUNTIME DESTINATION bin)

# Add a target which builds all utilities
add_custom_target(utils DEPENDS ${UTILS})

