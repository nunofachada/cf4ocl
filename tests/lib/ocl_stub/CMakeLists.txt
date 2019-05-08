# Sources required for building OpenCL stub library
set(SRC ocl_commandqueue.c ocl_context.c ocl_device.c ocl_env.c
        ocl_event.c ocl_platform.c ocl_program.c ocl_kernel.c utils.c
        ocl_memobject.c ocl_enqueue.c ocl_buffer.c ocl_image.c
        ocl_sampler.c)

# Add library
add_library(OpenCL_STUB_LIB ${SRC})

# Dependencies for the OpenCL stub library
target_link_libraries(OpenCL_STUB_LIB ${GLIB_LDFLAGS})