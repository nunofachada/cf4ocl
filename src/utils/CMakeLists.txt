# Build utils?
option(BUILD_UTILS "Build and install utilities?" ON)

# Stop processing if utils are not to be built
if (NOT ${BUILD_UTILS})
	return()
endif()

# List of utilities
set(UTILS
	ccl_devinfo  # Device information
	ccl_c)       # Kernel compiler

# Add a target for each utility
foreach(UTIL ${UTILS})
	add_executable(${UTIL} ${UTIL}.c)
	target_link_libraries(${UTIL} ${PROJECT_NAME})
endforeach(UTIL)

# Set utils version
set_target_properties(${UTILS} PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION})

# How to install utils
install(TARGETS ${UTILS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Add a target which builds all utilities
add_custom_target(utils DEPENDS ${UTILS})

