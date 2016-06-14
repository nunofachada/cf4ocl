Tutorial {#tut}
========

### Introduction

This tutorial is based on the `canon` example available in the
examples folder. The goal is to add two vectors, `a` and `b`, as well as
a constant `d`, and save the result in a third vector, `c`. The OpenCL
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

### Getting started

The _cf4ocl_ header should be included at the beginning of the `mysum.c`
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
$ gcc `pkg-config --cflags cf4ocl2` mysum.c -o mysum `pkg-config --libs cf4ocl2`
$ ./mysum
~~~~~~~~~~~~~~~

With Clang the command is the same, just replace `gcc` with `clang`.

### Host and device buffers

The goal of the program is to sum two vectors and a constant. Let's
declare two host vectors with some values, a third host vector which
will hold the result, three device buffers and the constant.

~~~~~~~~~~~~~~~{.c}

#define VECSIZE 8
#define SUM_CONST 3

    //...

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;

~~~~~~~~~~~~~~~

It's necessary to instantiate the device buffers, of which `a` and `b`
will be initialized with the values in the respective host vectors:

~~~~~~~~~~~~~~~{.c}

    //...

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

    //...

~~~~~~~~~~~~~~~

Don't forget the destructors at the end:

~~~~~~~~~~~~~~~{.c}

    //...

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
    cl_uint d = SUM_CONST;

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

### Creating the command queue

The ::ccl_queue_new() constructor provides the simplest way to create a
command queue. However, command queue creation requires a device. At
this time, only the context has the memory address of the selected
device. To get a pointer to the device, one can use the
::ccl_context_get_device() function:

~~~~~~~~~~~~~~~{.c}
    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice* dev = NULL;
    //...

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, NULL);
    if (dev == NULL) exit(-1);
~~~~~~~~~~~~~~~

There's no need to release the device object. It will be automatically
released when the context is destroyed, in accordance to the
@ref ug_new_destroy "new/destroy" rule.

Now we can create the command queue. We don't require any special
queue properties for now, so we pass `0` in the third argument:

~~~~~~~~~~~~~~~{.c}
    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice* dev = NULL;
    CCLQueue* queue = NULL;
    //...

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, NULL);
    if (queue == NULL) exit(-1);
    //...

    /* Destroy cf4ocl wrappers. */
    //...
    ccl_queue_destroy(queue);
~~~~~~~~~~~~~~~

Both ::ccl_context_get_device() and ::ccl_queue_new() expect an error
handling object as the last argument. By passing `NULL` we must rely
on the return value of these functions in order to check for errors.
Here's the complete program so far:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * queue = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, NULL);
    if (dev == NULL) exit(-1);

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, NULL);
    if (queue == NULL) exit(-1);

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
    ccl_queue_destroy(queue);
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~

Compile it and run it. As expected, nothing special happens yet.

### Creating and building the program

_cf4ocl_ provides several constructors for creating
@ref CCL_PROGRAM_WRAPPER "program objects". When a single OpenCL C
kernel source file is involved, the most adequate constructor is
::ccl_program_new_from_source_file():

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    CCLProgram* prg = NULL;

    //...
    /* Create program. */
    prg = ccl_program_new_from_source_file(ctx, "mysum.cl", NULL);
    if (prg == NULL) exit(-1);
    //...

    /* Destroy cf4ocl wrappers. */
    ccl_program_destroy(prg);
    //...

~~~~~~~~~~~~~~~

Building the program is just as easy. For this purpose, we use the
::ccl_program_build() function which returns `CL_TRUE` if the build is
successful or `CL_FALSE` otherwise:

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    cl_bool status;

    //...
    /* Build program. */
    status = ccl_program_build(prg, NULL, NULL);
    if (!status) exit(-1);

~~~~~~~~~~~~~~~

Here's the current state of our program:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * queue = NULL;
    CCLProgram* prg = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;
    cl_bool status;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, NULL);
    if (dev == NULL) exit(-1);

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, NULL);
    if (queue == NULL) exit(-1);

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

    /* Create program. */
    prg = ccl_program_new_from_source_file(ctx, "mysum.cl", NULL);
    if (prg == NULL) exit(-1);

    /* Build program. */
    status = ccl_program_build(prg, NULL, NULL);
    if (!status) exit(-1);

    /* Destroy cf4ocl wrappers. */
    ccl_program_destroy(prg);
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_queue_destroy(queue);
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~

Compile and run it. Don't forget to put the `mysum.cl` file containing
the kernel source code in the same folder, otherwise the program will
not be successfully created.

**Pro tip:** To get the build log automatically printed to screen, set
the `G_MESSAGES_DEBUG` environment variable to `cf4ocl2`. For example:

~~~~~~~~~~~~~~~
$ G_MESSAGES_DEBUG=cf4ocl2 ./mysum
~~~~~~~~~~~~~~~

Add some junk to the kernel source code, run our program and check the
build log.

### Setting kernel arguments and running the program

_cf4ocl_ greatly simplifies the execution of OpenCL programs. Instead
of creating a kernel with clCreateKernel(), setting kernel arguments
one-by-one with clSetKernelArg(), and finally executing the kernel with
clEnqueueNDRangeKernel(), _cf4ocl_ allows client code to do this using
a single function:

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    size_t gws = VECSIZE;
    CCLEvent* evt = NULL;

    //...
    /* Set kernel arguments and run kernel. */
    evt = ccl_program_enqueue_kernel(prg, "sum", queue, 1, &gws,
        NULL, NULL, NULL, a, b, c, ccl_arg_priv(d, cl_uint),
        ccl_arg_priv(gws, cl_uint), NULL);
    if (!evt) exit(-1);

~~~~~~~~~~~~~~~

Buffer, image and sampler objects can be passed directly as kernel
arguments. Local and private variables are passed using the
::ccl_arg_local() and ::ccl_arg_priv() macros, respectively.

A local work size vector is expected as the 6th argument to
::ccl_program_enqueue_kernel(). In this example, we pass `NULL`, which
means that the local work size is to be automatically determined by the
OpenCL implementation (as specified in the clEnqueueNDRangeKernel()
documentation). Often we need more control over this value, because
OpenCL implementations don't let us know what local work size was
effectively used. It can be a bit of a chore to determine a local work
size, especially when multiple dimensions are involved. Among other
things, it's necessary to check kernel and device limits. The
::ccl_kernel_suggest_worksizes() is a very versatile function which can
help in this regard.

While the ::ccl_program_enqueue_kernel() simplifies executing a kernel
(including setting its arguments), _cf4ocl_ provides additional
functions which allow client code to have finer control over this
process.

### Checking results

To check the results of the kernel execution, it's necessary to first
read the `c` output buffer from the device:

~~~~~~~~~~~~~~~{.c}

    /* Read the output buffer from the device. */
    evt = ccl_buffer_enqueue_read(c, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_c, NULL, NULL);
    if (!evt) exit(-1);

~~~~~~~~~~~~~~~

Now we can check the results:

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    int i;
    //...

    /* Check for errors. */
    for (i = 0; i < VECSIZE; ++i) {
        if (vec_c[i] != vec_a[i] + vec_b[i] + c) {
            fprintf(stderr, "Unexpected results.\n");
            exit(-1);
        }
    }
    /* No errors found. */
    printf("Results OK!\n");

~~~~~~~~~~~~~~~

We now have a fully working OpenCL program, simplified with _cf4ocl_:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * queue = NULL;
    CCLProgram * prg = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;
    size_t gws = VECSIZE;
    cl_bool status;
    CCLEvent * evt = NULL;
    int i;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(NULL);
    if (ctx == NULL) exit(-1);

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, NULL);
    if (dev == NULL) exit(-1);

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, NULL);
    if (queue == NULL) exit(-1);

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

    /* Create program. */
    prg = ccl_program_new_from_source_file(ctx, "mysum.cl", NULL);
    if (prg == NULL) exit(-1);

    /* Build program. */
    status = ccl_program_build(prg, NULL, NULL);
    if (!status) exit(-1);

    evt = ccl_program_enqueue_kernel(prg, "sum", queue, 1, NULL, &gws,
        NULL, NULL, NULL, a, b, c, ccl_arg_priv(d, cl_uint),
        ccl_arg_priv(gws, cl_uint), NULL);
    if (!evt) exit(-1);

    /* Read the output buffer from the device. */
    evt = ccl_buffer_enqueue_read(c, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_c, NULL, NULL);
    if (!evt) exit(-1);

    /* Some OpenCL implementations don't respect the blocking read,
     * so this guarantees that the read is effectively finished. */
    status = ccl_queue_finish(queue, NULL);
    if (!status) exit(-1);

    /* Check for errors. */
    for (i = 0; i < VECSIZE; ++i) {
        if (vec_c[i] != vec_a[i] + vec_b[i] + d) {
            fprintf(stderr, "Unexpected results.\n");
            exit(-1);
        }
    }
    /* No errors found. */
    printf("Results OK!\n");

   /* Destroy cf4ocl wrappers. */
    ccl_program_destroy(prg);
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_queue_destroy(queue);
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~

### A better way to check for errors

Our program may be correctly implemented, but a number of OpenCL errors
can still occur. Checking the return values of _cf4ocl_ functions
allows us only to determine that something went wrong, not what went
wrong. Fortunately, all error-throwing _cf4ocl_ functions accept the
memory location of [GError](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#GError)
error handling object, usually the last function argument. If given,
this object will be populated with an error code and an error domain, as
well as an error message, if an error occurs. In our program we're
passing `NULL` where the `GError` object memory location was expected,
so no information about errors is made available to the caller. To
change the way we're handling errors, we must first declare a `GError`
object, and initialize it to `NULL`:

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    GError * err = NULL;
~~~~~~~~~~~~~~~

Then, we should pass the memory location of this object to all _cf4ocl_
error-throwing functions, e.g.:

~~~~~~~~~~~~~~~{.c}
    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(&err);
    //...

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, &err);
    //...

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, &err);
    //...
~~~~~~~~~~~~~~~

Now we can check this object after _cf4ocl_ function calls. Let's create
a small macro to do so:

~~~~~~~~~~~~~~~{.c}
#define CHECK_ERROR(err) \
    if (err != NULL) { fprintf(stderr, "\n%s\n", err->message); exit(-1); }
~~~~~~~~~~~~~~~

We can also remove the `status` and `evt` variables, because we don't
rely on them anymore for error checking. Here's the complete program,
with more informative error checking:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3
#define CHECK_ERROR(err) \
    if (err != NULL) { fprintf(stderr, "\n%s\n", err->message); exit(-1); }

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * queue = NULL;
    CCLProgram * prg = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;
    size_t gws = VECSIZE;
    int i;
    GError * err = NULL;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(&err);
    CHECK_ERROR(err);

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, &err);
    CHECK_ERROR(err);

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, 0, &err);
    CHECK_ERROR(err);

    /* Instantiate and initialize device buffers. */
    a = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_a, &err);
    CHECK_ERROR(err);

    b = ccl_buffer_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        VECSIZE * sizeof(cl_uint), vec_b, &err);
    CHECK_ERROR(err);

    c = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    /* Create program. */
    prg = ccl_program_new_from_source_file(ctx, "mysum.cl", &err);
    CHECK_ERROR(err);

    /* Build program. */
    ccl_program_build(prg, NULL, &err);
    CHECK_ERROR(err);

    ccl_program_enqueue_kernel(prg, "sum", queue, 1, NULL, &gws,
        NULL, NULL, &err, a, b, c, ccl_arg_priv(d, cl_uint),
        ccl_arg_priv(gws, cl_uint), NULL);
    CHECK_ERROR(err);

    /* Read the output buffer from the device. */
    ccl_buffer_enqueue_read(c, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_c, NULL, &err);
    CHECK_ERROR(err);

    /* Some OpenCL implementations don't respect the blocking read,
     * so this guarantees that the read is effectively finished. */
    ccl_queue_finish(queue, &err);
    CHECK_ERROR(err);

    /* Check for errors. */
    for (i = 0; i < VECSIZE; ++i) {
        if (vec_c[i] != vec_a[i] + vec_b[i] + d) {
            fprintf(stderr, "Unexpected results.\n");
            exit(-1);
        }
    }
    /* No errors found. */
    printf("Results OK!\n");

   /* Destroy cf4ocl wrappers. */
    ccl_program_destroy(prg);
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_queue_destroy(queue);
    ccl_context_destroy(ctx);

    return 0;
}
~~~~~~~~~~~~~~~

The error checking strategy in our program is just an example. Client
code can implement any strategy. More information about this topic is
available in the @ref ug_errorhandle "user guide".

### Profiling your program the easy way

Profiling OpenCL code by hand, i.e. by gathering and processing
information about all `cl_event` objects, is an extremely verbose and
error-prone process. However, it comes for free with _cf4ocl_. Well,
mostly. The first change we must perform on our program is to enable
profiling when the command queue is created:

~~~~~~~~~~~~~~~{.c}

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);

~~~~~~~~~~~~~~~

Additionally, we copied host vectors to device implicitly during device
buffer creation. These implicit transfers don't produce OpenCL events,
and are thus unavailable for profiling. As such, we must make these
operations explicit:

~~~~~~~~~~~~~~~{.c}

    /* Instantiate device buffers. */
    a = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    b = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    //...

    /* Initialize device buffers. */
    ccl_buffer_enqueue_write(a, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_a, NULL, &err);
    CHECK_ERROR(err);

    ccl_buffer_enqueue_write(b, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_b, NULL, &err);
    CHECK_ERROR(err);

~~~~~~~~~~~~~~~

We can now profile our program using functionality provided by the
@ref CCL_PROFILER "profiler module":

~~~~~~~~~~~~~~~{.c}

    /* Variables. */
    //...
    CCLProf* prof = NULL;

    //...

    /* Perform profiling. */
    prof = ccl_prof_new();
    ccl_prof_add_queue(prof, "queue1", queue);
    ccl_prof_calc(prof, &err);
    CHECK_ERROR(err);

    /* Show profiling info. */
    ccl_prof_print_summary(prof);

    /* Destroy profiler object. */
    ccl_prof_destroy(prof);

~~~~~~~~~~~~~~~

Our final program, with error checking and profiling, is as follows:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>

#define VECSIZE 8
#define SUM_CONST 3
#define CHECK_ERROR(err) \
    if (err != NULL) { fprintf(stderr, "\n%s\n", err->message); exit(-1); }

int main() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * queue = NULL;
    CCLProgram * prg = NULL;
    CCLBuffer * a = NULL, * b = NULL, * c = NULL;
    cl_uint vec_a[VECSIZE] = {0, 1, 2, 3, 4, 5, 6, 7};
    cl_uint vec_b[VECSIZE] = {3, 2, 1, 0, 1, 2, 3, 4};
    cl_uint vec_c[VECSIZE];
    cl_uint d = SUM_CONST;
    size_t gws = VECSIZE;
    int i;
    GError * err = NULL;
    CCLProf* prof = NULL;

    /* Create context with user selected device. */
    ctx = ccl_context_new_from_menu(&err);
    CHECK_ERROR(err);

    /* Get the selected device. */
    dev = ccl_context_get_device(ctx, 0, &err);
    CHECK_ERROR(err);

    /* Create a command queue. */
    queue = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    CHECK_ERROR(err);

    /* Instantiate device buffers. */
    a = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    b = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    c = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
        VECSIZE * sizeof(cl_uint), NULL, &err);
    CHECK_ERROR(err);

    /* Initialize device buffers. */
    ccl_buffer_enqueue_write(a, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_a, NULL, &err);
    CHECK_ERROR(err);

    ccl_buffer_enqueue_write(b, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_b, NULL, &err);
    CHECK_ERROR(err);

    /* Create program. */
    prg = ccl_program_new_from_source_file(ctx, "mysum.cl", &err);
    CHECK_ERROR(err);

    /* Build program. */
    ccl_program_build(prg, NULL, &err);
    CHECK_ERROR(err);

    ccl_program_enqueue_kernel(prg, "sum", queue, 1, NULL, &gws,
        NULL, NULL, &err, a, b, c, ccl_arg_priv(d, cl_uint),
        ccl_arg_priv(gws, cl_uint), NULL);
    CHECK_ERROR(err);

    /* Read the output buffer from the device. */
    ccl_buffer_enqueue_read(c, queue, CL_TRUE, 0,
        VECSIZE * sizeof(cl_uint), vec_c, NULL, &err);
    CHECK_ERROR(err);

    /* Some OpenCL implementations don't respect the blocking read,
     * so this guarantees that the read is effectively finished. */
    ccl_queue_finish(queue, &err);
    CHECK_ERROR(err);

    /* Check for errors. */
    for (i = 0; i < VECSIZE; ++i) {
        if (vec_c[i] != vec_a[i] + vec_b[i] + d) {
            fprintf(stderr, "Unexpected results.\n");
            exit(-1);
        }
    }
    /* No errors found. */
    printf("Results OK!\n");

    /* Perform profiling. */
    prof = ccl_prof_new();
    ccl_prof_add_queue(prof, "queue1", queue);
    ccl_prof_calc(prof, &err);
    CHECK_ERROR(err);

    /* Show profiling info. */
    ccl_prof_print_summary(prof);

    /* Destroy profiler object. */
    ccl_prof_destroy(prof);

   /* Destroy cf4ocl wrappers. */
    ccl_program_destroy(prg);
    ccl_buffer_destroy(c);
    ccl_buffer_destroy(b);
    ccl_buffer_destroy(a);
    ccl_queue_destroy(queue);
    ccl_context_destroy(ctx);

    return 0;
}

~~~~~~~~~~~~~~~

Compile and run the program. A profiling summary will be printed on the
screen, something like:

@verbatim

 Aggregate times by event  :
   ------------------------------------------------------------------
   | Event name                     | Rel. time (%) | Abs. time (s) |
   ------------------------------------------------------------------
   | NDRANGE_KERNEL                 |       64.8583 |    1.7993e-05 |
   | WRITE_BUFFER                   |       31.8254 |    8.8290e-06 |
   | READ_BUFFER                    |        3.3163 |    9.2000e-07 |
   ------------------------------------------------------------------
                                    |         Total |    2.7742e-05 |
                                    ---------------------------------
 Event overlaps            : None

@endverbatim

While this profiling summary is useful, the
@ref CCL_PROFILER "profiler module" can do much more.
