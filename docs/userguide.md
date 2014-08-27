User guide
==========

[TOC]

# Overview {#ug_overview}

The C Framework for OpenCL, cf4ocl, is a cross-platform pure C99
object-oriented framework for developing and benchmarking OpenCL
projects in the C programming language. It aims to:

* Promote the rapid development of OpenCL programs in C.
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

### Library organization

* Wrappers, which wrap OpenCL objects and functions, and provide some
utility functions.
* Profiler
* Device selector - an extension to the Context wrapper
* Device query - used by ccl_devinfo util

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

Error throwing _cf4ocl_ functions provide two methods for client-side
error handling:

1. Using the return value.
2. Passing a pointer to a
[GError](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#GError)
object.

The first method consists of analysing the return value of a function.
Error throwing functions which return a pointer will return `NULL` if
an error occurs. The remaining error throwing functions return
`CL_FALSE` if an error occurs (or `CL_TRUE` otherwise). Client code can
check for errors by looking for `NULL` or `CL_FALSE` return values,
depending on the function. This error handling method does not provide
information on which exact error occured. For example:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
cl_bool ret_status;
...
ret_status = ccl_program_new_from_source_file(ctx, "program.cl", NULL);
if (!ret_status) {
	fprintf(stderr, "An error ocurred");
	exit(-1);
}
~~~~~~~~~~~~~~~

The second method is more flexible. The `GError` object should be
initialized to `NULL`, and is then passed as the last argument to the
function.

TO DO

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
