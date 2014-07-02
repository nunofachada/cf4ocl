# Sources required for building OpenCL stub library
set(SRC ocl_commandqueue.c ocl_context.c ocl_device.c ocl_env.c 
		ocl_event.c ocl_platform.c ocl_program.c ocl_kernel.c utils.c)

# Add library
add_library(OpenCL_STUB_LIB ${SRC})
