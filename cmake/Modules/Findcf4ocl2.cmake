# - Try to find cf4ocl2
# Once done, this will define
#
#  CF4OCL2_FOUND - system has cf4ocl2
#  CF4OCL2_INCLUDE_DIRS - the cf4ocl2 include directories
#  CF4OCL2_LIBRARIES - link these to use cf4ocl2

include(LibFindMacros)

# Include dir
find_path(CF4OCL2_INCLUDE_DIR cf4ocl2.h)

# Finally the library itself
find_library(CF4OCL2_LIBRARY cf4ocl2)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(CF4OCL2_PROCESS_INCLUDES CF4OCL2_INCLUDE_DIR)
set(CF4OCL2_PROCESS_LIBS CF4OCL2_LIBRARY)
libfind_process(CF4OCL2)
