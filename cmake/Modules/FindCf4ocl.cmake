# - Try to find cf4ocl
# Once done, this will define
#
#  CF4OCL_FOUND - system has cf4ocl
#  CF4OCL_INCLUDE_DIRS - the cf4ocl include directories
#  CF4OCL_LIBRARIES - link these to use cf4ocl

include(LibFindMacros)

# Include dir
find_path(CF4OCL_INCLUDE_DIR cf4ocl.h)

# Finally the library itself
find_library(CF4OCL_LIBRARY cf4ocl)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(CF4OCL_PROCESS_INCLUDES CF4OCL_INCLUDE_DIR)
set(CF4OCL_PROCESS_LIBS CF4OCL_LIBRARY)
libfind_process(CF4OCL)
