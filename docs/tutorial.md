Tutorial {#tut}
========

This tutorial is based on the `canon` example available in the
examples folder. The goal is to add two vectors, `a` and `b`, as well as
a constant `d`, and save the result in a third vector, 'c'. The OpenCL
kernel which performs this operation is given in the following code:

~~~~~~~~~~~~~~~{.c}
__kernel void sum(__global const uint *a, __global const uint *b,
	__global uint * c, uint d, uint buf_size) {

	/* Get global ID. */
	uint gid = get_global_id(0);

	/* Only perform sum if this workitem is within the size of the
	 * vector. */
	if (gid < buf_size)
		c[gid] = a[gid] + b[gid] + d;
}
~~~~~~~~~~~~~~~

For the purpose of this tutorial, we'll assume the kernel code is in a
file called `mysum.cl`, and the host code in `mysum.c`.

The _cf4ocl_ header should be included at the beggining of the `mysum.c`
file:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>
~~~~~~~~~~~~~~~

The next step is to get an context with an OpenCL device where we can
perform our computation. _cf4ocl_ has several constructor functions for
creating contexts with different types of devices, some very simple,
some very flexible. For example, ::ccl_context_new_from_menu() lets
the user select the OpenCL device if more than one is available in the
system, and returns a context containing the selected device. Let's use
it:

~~~~~~~~~~~~~~~{.c}

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;

    /* Code. */
    ctx = ccl_context_new_from_menu(NULL);

~~~~~~~~~~~~~~~

Where we pass `NULL` we could have passed an error management object,
which we'll discuss in detail further ahead. Error-throwing _cf4ocl_
signal errors in two ways: 1) using the return value; and, 2) populating
the error management object. In this case, because we're not passing
this object, we have to rely on the return value to check for errors. A
`NULL` return value indicates an error in all _cf4ocl_ constructors:

~~~~~~~~~~~~~~~{.c}
    if (ctx == NULL) exit(-1);
~~~~~~~~~~~~~~~

In _cf4ocl_, all objects created with `new` constructors must be
release using the respective `destroy` destructors. For contexts, this
is the ::ccl_context_destroy() function:

~~~~~~~~~~~~~~~{.c}
    /* Destroy context wrapper. */
    ccl_context_destroy(ctx);
~~~~~~~~~~~~~~~

We now have compilable, leak-free _cf4ocl_ program, although it doesn't
do very much yet:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;

    /* Code. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Destroy context wrapper. */
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~

We can compile the program with `gcc` (or `clang`), and run it:
~~~~~~~~~~~~~~~
$ gcc `pkg-config --cflags glib-2.0` mysum.c -o mysum -lcf4ocl2 `pkg-config --libs glib-2.0` -lOpenCL
$ ./mysum
~~~~~~~~~~~~~~~

The goal of the program is to sum two vectors and a constant. Let's
declare two host vectors with some values, a third host vector which
will hold the result, three device buffers and the constant.

~~~~~~~~~~~~~~~{.c}

#define VECSIZE 8
#define SUM_CONST 3

    ...

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    const cl_uint d = SUM_CONST;

~~~~~~~~~~~~~~~

It's necessary to instantiate the device buffers, of which `a` and `b`
will be initialized with the values in the respective host vectors:

~~~~~~~~~~~~~~~{.c}

    ...

    /* Code. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Instantiate and initialize device buffers. */
    a = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_a, NULL);
    if (a == NULL) exit(-1);

    b = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_b, NULL);
    if (b == NULL) exit(-1);

    c = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, NULL);
    if (c == NULL) exit(-1);

    ....

~~~~~~~~~~~~~~~

Don't forget the destructors at the end:

~~~~~~~~~~~~~~~{.c}

    ...

    /* Destroy cf4ocl wrappers. */
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_context_destroy(ctx);

    return 0;

~~~~~~~~~~~~~~~

The complete program so far, which can be compiled and executed. Still
doesn't do anything useful, but we're getting close.

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    const cl_uint d = SUM_CONST;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Instantiate and initialize device buffers. */
    a = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_a, NULL);
    if (a == NULL) exit(-1);

    b = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_b, NULL);
    if (b == NULL) exit(-1);

    c = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, NULL);
    if (c == NULL) exit(-1);

    /* Destroy cf4ocl wrappers. */
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~


