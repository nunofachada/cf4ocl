# Build examples?
option(BUILD_EXAMPLES "Build examples?" ON)

# Stop processing if examples are not to be built
if (NOT ${BUILD_EXAMPLES})
    return()
endif()

# How to stringify kernels
if (WIN32 OR APPLE)
    set(EXAMPLES_STRINGIFY_DEFAULT "hex")
else()
    set(EXAMPLES_STRINGIFY_DEFAULT "text")
endif()
set(EXAMPLES_STRINGIFY ${EXAMPLES_STRINGIFY_DEFAULT}
    CACHE
    STRING "Example kernel stringification: hex (cross-platform, ugly) or text (requires GNU sed and awk)")
set_property(CACHE EXAMPLES_STRINGIFY PROPERTY STRINGS "hex" "text")

# Examples without OpenCL kernel code
set(EXAMPLES_NOCL device_filter image_fill list_devices)

# Examples to be configured with OpenCL kernel code
set(EXAMPLES_CL image_filter ca canon)

# Specify location of stb headers for PNG load/save
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Configure examples using OpenCL kernel code
foreach(EXAMPLE ${EXAMPLES_CL})

    # Kernel stringification
    if (${EXAMPLES_STRINGIFY} STREQUAL "hex")
        # Hex stringification, cross-platform but ugly

        # Load kernel code into string
        file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.cl ${EXAMPLE}_RAW HEX)

        # Escape hex character codes to be recognized as such within C source
        string(REGEX REPLACE "(..)" "\\\\x\\1" ${EXAMPLE}_KERNEL_SRC
            ${${EXAMPLE}_RAW})

        # Pad string with double quotes
        set(${EXAMPLE}_KERNEL_SRC "\"${${EXAMPLE}_KERNEL_SRC}\"")

    else()
        # Text stringification, better looking but requires awk and sed

        # Execute stringification script
        execute_process(
            COMMAND "${CMAKE_SOURCE_DIR}/auxbuild/stringify.sh"
            INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.cl"
            OUTPUT_VARIABLE ${EXAMPLE}_KERNEL_SRC)

    endif()

    # Configure example
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}.c
        ${CMAKE_BINARY_DIR}/examples/${EXAMPLE}.c @ONLY)

    # Add target for current example
    add_executable(${EXAMPLE} ${CMAKE_BINARY_DIR}/examples/${EXAMPLE}.c)
    target_link_libraries(${EXAMPLE} ${PROJECT_NAME})

endforeach()

# Add a target for examples without OpenCL kernel code
foreach(EXAMPLE ${EXAMPLES_NOCL})
    add_executable(${EXAMPLE} ${EXAMPLE}.c)
    target_link_libraries(${EXAMPLE} ${PROJECT_NAME})
endforeach()

# The stb_image.h included in image_filter throws some warnings,
# this avoids them.
if((${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    OR (${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))

    set_target_properties(image_filter PROPERTIES
        COMPILE_FLAGS "-Wno-type-limits")

endif()

# Add a target which builds all samples
add_custom_target(examples DEPENDS ${EXAMPLES_NOCL} ${EXAMPLES_CL})
