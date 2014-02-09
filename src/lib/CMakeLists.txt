# Sources required for building library
set(SRC clerrors.c clprofiler.c clutils.c)

# Add library
add_library(cf4ocl SHARED ${SRC})

# This target is just an alias for cf4ocl
add_custom_target(lib DEPENDS cf4ocl)
