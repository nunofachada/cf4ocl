User guide
==========

[TOC]

# Overview {#ug_overview}

The C Framework for OpenCL, cf4ocl, is a cross-platform pure C99
object-oriented framework for developing and benchmarking OpenCL
projects in C and C++. It aims to:

* Promote the rapid development of OpenCL programs in C and C++.
* Assist in the benchmarking of OpenCL events, such as kernel execution
and data transfers.
* Simplify the analysis of the OpenCL environment and of kernel
requirements.

## Features {#ug_features}

* Object-oriented interface to the OpenCL API
  * New/destroy functions, no direct memory alloc/free
  * Easy (and extensible) device selection
  * Simple event dependency management
  * User-friendly error management
* OpenCL version independent
* Integrated profiling

# Using the library {#ug_libray}

## Basics {#ug_basics}

### Framework organization

The _cf4ocl_ library offers an object-oriented interface to the OpenCL
API using wrapper classes and methods (or structs and functions, in C
terms). Each type of OpenCL object is wrapped in a _cf4ocl_ class, as
shown in the following table:

| Class         | [OpenCL type]    | _cf4ocl_ wrapper |
| ------------- | ---------------- | ---------------- |
| Platform      | cl_platform_id   | ::CCLPlatform*   |
| Device        | cl_device_id     | ::CCLDevice*     |
| Context       | cl_context       | ::CCLContext*    |
| Command queue | cl_command_queue | ::CCLQueue*      |
| Program       | cl_program       | ::CCLProgram*    |
| Kernel        | cl_kernel        | ::CCLKernel*     |
| Event         | cl_event         | ::CCLEvent*      |
| MemObject     | cl_mem           | ::CCLMemObj*     |
| Buffer        | cl_mem           | ::CCLBuffer*     |
| Image         | cl_mem           | ::CCLImage*      |
| Sampler       | cl_sampler       | ::CCLSampler*    |

Each wrapper class has several methods (functions). Some of these
directly wrap OpenCL functions (::ccl_buffer_enqueue_copy()), while
others perform a number of OpenCL operations (e.g.
::ccl_kernel_set_args_and_enqueue_ndrange()).

The device and context wrappers classes are augmented by the
@ref DEVICE_SELECTOR "device selector module", which allows to
automatically select devices using selected filters.

_cf4ocl_ also provides a fully integrated profiling class, ::CCLProf.
When the OpenCL computations are over, the command queues (i.e., their
wrapper objects) used in the computation can be added to the profiling
object, and a complete profiling analysis can be performed.

Finally, the framework also provides two utilities:

* `ccl_devinfo`
* `ccl_kerninfo`

### The new/destroy rule {#ug_new_destroy}

The _cf4ocl_ constructors and destructors have `new` and `destroy` in
their name, respectively. In _cf4ocl_, the new/destroy rule states
the following:

_For each invoked constructor, the respective destructor must also be
invoked._

This might seem obvious, but in many instances several objects are
obtained using other (non-constructor) methods during the course of a
program. These objects are automatically released and should not be
destroyed by client code.

For example, it is possible to get a device belonging to a context
using the ::ccl_context_get_device() function:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLDevice* dev;
...
ctx = ccl_context_new_any(NULL);
...
dev = ccl_context_get_device(ctx, 0, NULL);
~~~~~~~~~~~~~~~

The returned device wrapper object will be freed when the context
wrapper object is destroyed; as such, there is no need to free the
device wrapper object:

~~~~~~~~~~~~~~~{.c}
ccl_context_destroy(ctx);
~~~~~~~~~~~~~~~

### Getting info about OpenCL objects {#ug_getinfo}

The `ccl_<object>_get_info_<scalar|array>()` macros can be used to get
information about OpenCL objects. Use the `array` version when the
expected return value is a pointer or array, or the `scalar` version
otherwise (e.g. when the expected return value is primitive or scalar
type).

For example, to get the name and the number of compute cores on a device:

~~~~~~~~~~~~~~~{.c}
CCLDevice* dev;
char* name;
cl_uint n_cores;
...
name = ccl_device_get_info_array(dev, CL_DEVICE_NAME, char*, NULL);
n_cores = ccl_device_get_info_scalar(dev, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, NULL);
~~~~~~~~~~~~~~~

The `ccl_<object>_get_info()` macros serve more specific scenarios, and
are likely to be used less often. These macros return a
::CCLWrapperInfo* object, which contains two public fields:

* `value` - A pointer to the requested value as returned by the OpenCL
clGet<Object>Info() functions.
* `size` - The size in bytes of the value pointed to by the `value`
field.

To use the value, a cast should be performed on the `value` field to
convert it to the required type (which is what the
`ccl_<object>_get_info_<scalar|array>()` macros do).

The values and objects returned by these macros are automatically
released when the respective wrapper object is destroyed and should
never be directly freed by client code. In the previous example,
destroying the device wrapper will also destroy any requested info:

~~~~~~~~~~~~~~~{.c}
ccl_device_destroy(dev);
~~~~~~~~~~~~~~~

### Error handling {#ug_errorhandle}

Error-reporting _cf4ocl_ functions provide two methods for client-side
error handling:

1. The return value.
2. [GError](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#GError)-based
error reporting.

The first method consists of analysing the return value of a function.
Error throwing functions which return a pointer will return `NULL` if
an error occurs. The remaining error-reporting functions return
`CL_FALSE` if an error occurs (or `CL_TRUE` otherwise). Client code can
check for errors by looking for `NULL` or `CL_FALSE` return values,
depending on the function. This error handling method does not provide
information on which exact error occured. For example:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
...
prg = ccl_program_new_from_source_file(ctx, "program.cl", NULL);
if (!prg) {
    fprintf(stderr, "An error ocurred");
    exit(-1);
}
~~~~~~~~~~~~~~~

The second method uses the [GLib Error Reporting](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html)
approach and is more flexible. A `GError` object is initialized to
`NULL`, and a pointer to it is passed as the last argument to the
function being called. If the `GError` object is still `NULL` after the
function call, no error has occurred. Otherwise, an error occurred
and it is possible to get a user-friendly error message:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
GError* err = NULL;
...
prg = ccl_program_new_from_source_file(ctx, "program.cl", &err);
if (err) {
    fprintf(stderr, "%s", err->message);
    exit(-1);
}
~~~~~~~~~~~~~~~

An error domain and error code are also available in the `GError`
object. The domain indicates the module the error-reporting function is
located in, while code indicates the specific error that occurred.
Three kinds of domain can be returned by error-reporting _cf4ocl_
functions, each of them associated with distinct error codes:

| Domain          | Codes                                                             | Description                                           |
| --------------- | ----------------------------------------------------------------- | ----------------------------------------------------- |
| ::CCL_ERROR     | ::ccl_error_code enum                                             | Error in _cf4ocl_ not related with external libraries |
| ::CCL_OCL_ERROR | [cl.h](http://www.khronos.org/registry/cl/api/2.0/cl.h)           | Error in OpenCL function calls                        |
| A Glib domain   | [GLib-module](https://developer.gnome.org/glib/stable/) dependent | Error in GLib function call (file open/save, etc)     |

For example, if client code wants to act on different OpenCL error
codes, it still can:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLBuffer* buf;
GError* err = NULL;
...
buf = ccl_buffer_new(ctx, flags, size, host_ptr, &err);
if (err) {
    if (err->domain == CCL_OCL_ERROR) {
        /* Check if it's OpenCL error. */
        switch (err->code) {
            /* Do different things depending on OpenCL error code. */
            case CL_INVALID_VALUE:
                ...
            case CL_INVALID_BUFFER_SIZE:
                ...
            case CL_INVALID_HOST_PTR:
                ...
            ...
        }
    } else {
        ...
    }
}
~~~~~~~~~~~~~~~

Finally, if client code wants to continue execution after an error was
caught, it is mandatory to use the [g_clear_error()] function to free
the error object and reset its value to `NULL`. Not doing so is a bug,
especially if more error-reporting functions are to be called moving
forward. For example:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
GError* err = NULL;
...
prg = ccl_program_new_from_source_file(ctx, "program.cl", &err);
if (err) {
    /* Print the error message, but don't terminate program. */
    fprintf(stderr, "%s", err->message);
    g_clear_error(&err);
}
~~~~~~~~~~~~~~~

Even if the program terminates due to an error, the [g_clear_error()]
can be still be called to destroy the error object.

## Wrapper objects {#ug_wrappers}

* Platforms
* Devices
* Contexts
* Command queues
* Memory objects
  * Buffer objects
  * Image objects
* Sampler objects
* Program objects
* Kernel objects
* Event objects

## Device/context selection {#ug_devsel}

Using filters.

## Profiling {#ug_profiling}

Very easy.

# Using the utilities {#ug_utils}

## ccl_devinfo {#ug_devinfo}

Man page here.

## ccl_kerninfo {#ug_kerninfo}

Man page here.

# Advanced {#ug_advanced}

## Architecture {#ug_architecture}

### Class tree {#ug_classtree}

Show the class tree, maybe a dia diagram or own Doxygen diagram.

### OpenCL object/cf4ocl wrapper uniqueness {#ug_unique}

One-to-one.

## Build and install from source {#ug_buildinstall}

CMake style.

[GLib]: https://developer.gnome.org/glib/ "GLib"
[g_clear_error()]: https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#g-clear-error "g_clear_error()"
[OpenCL type]: http://www.khronos.org/registry/cl/sdk/2.0/docs/man/xhtml/abstractDataTypes.html "OpenCL types"
