# Sources required for building library
set(SRC clerrors.c clprofiler.c clutils.c)

# Add library
add_library(cf4ocl SHARED ${SRC})

# This target is just an alias for cf4ocl
add_custom_target(lib DEPENDS cf4ocl)

# Set library version
set_target_properties(cf4ocl PROPERTIES 
	VERSION 
		${cf4ocl_VERSION_MAJOR}.${cf4ocl_VERSION_MINOR}.${cf4ocl_VERSION_PATCH}
	SOVERSION
		${cf4ocl_VERSION_MAJOR})

# How to install lib
install(TARGETS cf4ocl LIBRARY DESTINATION lib)
