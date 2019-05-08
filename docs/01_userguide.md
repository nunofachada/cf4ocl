User guide {#ug}
==========

@brief _cf4ocl_ user guide.

[TOC]

The C Framework for OpenCL, _cf4ocl_, is a cross-platform pure C
object-oriented framework for accelerating the development of
[OpenCL](http://www.khronos.org/opencl/) projects in C, with support for C++.

This user guide is organized as follows. First, in section
@ref ug_library "Using the library", the @ref ug_basics "basics" of _cf4ocl_ are
introduced, followed by a more thorough discussion on how to use
@ref ug_wrappers "wrapper" and @ref ug_othermodules "other" library modules. The
utilities provided with _cf4ocl_ are presented in section
@ref ug_utils "Using the utilities". Advanced topics, such as the
@ref ug_architecture "wrapper architecure", are discussed in section
@ref ug_advanced "Advanced".

# Using the library {#ug_library}

## Basics {#ug_basics}

### Library organization {#ug_forg}

The _cf4ocl_ library offers an object-oriented interface to the OpenCL
API using wrapper classes and methods (or structs and functions, in C
terms), grouped into modules of the same name, as shown in the following
table:

| _cf4ocl_ module                         | _cf4ocl_ wrapper class | Wrapped OpenCL type |
| --------------------------------------- | ---------------------- | ------------------- |
| @ref CCL_PLATFORM_WRAPPER "Platform module" | ::CCLPlatform*         | cl_platform_id      |
| @ref CCL_DEVICE_WRAPPER "Device module"     | ::CCLDevice*           | cl_device_id        |
| @ref CCL_CONTEXT_WRAPPER "Context module"   | ::CCLContext*          | cl_context          |
| @ref CCL_QUEUE_WRAPPER "Queue module"       | ::CCLQueue*            | cl_command_queue    |
| @ref CCL_PROGRAM_WRAPPER "Program module"   | ::CCLProgram*          | cl_program          |
| @ref CCL_KERNEL_WRAPPER "Kernel module"     | ::CCLKernel*           | cl_kernel           |
| @ref CCL_EVENT_WRAPPER "Event module"       | ::CCLEvent*            | cl_event            |
| @ref CCL_MEMOBJ_WRAPPER "MemObj module"     | ::CCLMemObj*           | cl_mem              |
| @ref CCL_BUFFER_WRAPPER "Buffer module"     | ::CCLBuffer*           | cl_mem              |
| @ref CCL_IMAGE_WRAPPER "Image module"       | ::CCLImage*            | cl_mem              |
| @ref CCL_SAMPLER_WRAPPER "Sampler module"   | ::CCLSampler*          | cl_sampler          |

Some of the provided methods directly wrap OpenCL functions (e.g.
::ccl_buffer_enqueue_copy()), while others perform a number of OpenCL
operations in one function call
(e.g. ::ccl_kernel_set_args_and_enqueue_ndrange()). The wrapper classes
are organized in a hierarchical @ref ug_architecture "inheritance tree".

Additional modules are also available:

| _cf4ocl_ module                                | Description                                                                                        |
| ---------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| @ref CCL_DEVICE_SELECTOR "Device selector module"  | Automatically select devices using filters.                                                        |
| @ref CCL_DEVICE_QUERY "Device query module"        | Helpers for querying device information, mainly used by the @ref ccl_devinfo "ccl_devinfo" program. |
| @ref CCL_ERRORS "Errors module"                    | Convert OpenCL error codes into human-readable strings.                                            |
| @ref CCL_PLATFORMS "Platforms module"              | Management of the OpencL platforms available in the system.                                        |
| @ref CCL_PROFILER "Profiler module"                | Simple, convenient and thorough profiling of OpenCL events.                                        |

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

For example, it is possible to get a kernel belonging to a program
using the ::ccl_program_get_kernel() function:

~~~~~~~~~~~~~~~{.c}
CCLProgram* prg;
CCLKernel* krnl;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
prg = ccl_program_new_from_source_file(ctx, "myprog.cl", NULL);
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
krnl = ccl_program_get_kernel(prg, "someKernel", NULL);
~~~~~~~~~~~~~~~

The returned kernel wrapper object will be freed when the program
is destroyed; as such, there is no need to free it. Destroying the program will
suffice:

~~~~~~~~~~~~~~~{.c}
ccl_program_destroy(prg);
~~~~~~~~~~~~~~~

### Getting info about OpenCL objects {#ug_getinfo}

The `ccl_<class>_get_info_<scalar|array>()` macros can be used to get
information about OpenCL objects. Use the `array` version when the
expected return value is a pointer or array, or the `scalar` version
otherwise (e.g. when the expected return value is primitive or scalar
type).

For example, to get the name and the number of compute cores on a
device:

~~~~~~~~~~~~~~~{.c}
CCLDevice* dev;
char* name;
cl_uint n_cores;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
name = ccl_device_get_info_array(dev, CL_DEVICE_NAME, char*, NULL);
n_cores = ccl_device_get_info_scalar(dev, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, NULL);
~~~~~~~~~~~~~~~

The `ccl_<class>_get_info()` macros serve more specific scenarios, and
are likely to be used less often. These macros return a
::CCLWrapperInfo* object, which contains two public fields:

* `value` - A pointer to the requested value as returned by the OpenCL
`clGet*Info()` functions.
* `size` - The size in bytes of the value pointed to by the `value`
field.

To use the value, a cast should be performed on the `value` field to
convert it to the required type (which is what the
`ccl_<class>_get_info_<scalar|array>()` macros automatically do).

The values and objects returned by these macros are automatically
released when the respective wrapper object is destroyed and should
never be directly freed by client code.

### Error handling {#ug_errorhandle}

Error-reporting _cf4ocl_ functions provide two methods for client-side
error handling:

1. The return value.
2. ::CCLErr-based error reporting.

The first method consists of analysing the return value of a function.
Error-throwing functions which return a pointer will return `NULL` if an error
occurs. The remaining error-reporting functions return `CL_FALSE` if an error
occurs (or `CL_TRUE` otherwise). Client code can check for errors by looking for
`NULL` or `CL_FALSE` return values, depending on the function. This error
handling method does not provide additional information about the reported
error. For example:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
prg = ccl_program_new_from_source_file(ctx, "program.cl", NULL);
if (!prg) {
    fprintf(stderr, "An error ocurred");
    exit(-1);
}
~~~~~~~~~~~~~~~

The second method is more flexible. A ::CCLErr object is initialized to `NULL`,
and a pointer to it is passed as the last argument to the function being called.
If the ::CCLErr object is still `NULL` after the function call, no error has
occurred. Otherwise, an error occurred and it is possible to get a user-friendly
error message:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
CCLErr* err = NULL;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
prg = ccl_program_new_from_source_file(ctx, "program.cl", &err);
if (err) {
    fprintf(stderr, "%s", err->message);
    exit(-1);
}
~~~~~~~~~~~~~~~

An error domain and error code are also available in the ::CCLErr object. The
domain indicates the module or library in which the error was generated, while
the code indicates the specific error that occurred. Three kinds of domain can
be returned by error-reporting _cf4ocl_ functions, each of them associated with
distinct error codes:

| Domain          | Codes                                                                   | Description                                           |
| --------------- | ----------------------------------------------------------------------- | ----------------------------------------------------- |
| ::CCL_ERROR     | ::ccl_error_code enum                                                   | Error in _cf4ocl_ not related with external libraries |
| ::CCL_OCL_ERROR | [cl.h](https://github.com/KhronosGroup/OpenCL-Headers/blob/master/cl.h) | Error in OpenCL function calls                        |
| A GLib domain   | [GLib-module](https://developer.gnome.org/glib/stable/) dependent       | Error in GLib function call (file open/save, etc)     |

For example, it is possible for client code to act on different OpenCL errors:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLBuffer* buf;
CCLErr* err = NULL;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
buf = ccl_buffer_new(ctx, flags, size, host_ptr, &err);
if (err) {
    if (err->domain == CCL_OCL_ERROR) {
        /* Check if it's OpenCL error. */
        switch (err->code) {
            /* Do different things depending on OpenCL error code. */
            case CL_INVALID_VALUE:
                /* Handle invalid values */
            case CL_INVALID_BUFFER_SIZE:
                /* Handle invalid buffer sizes */
            case CL_INVALID_HOST_PTR:
                /* Handle invalid host pointer */

            /* Handle other OpenCL errors */

        }
    } else {
        /* Handle other errors */
    }
}
~~~~~~~~~~~~~~~

Finally, if client code wants to continue execution after an error was caught,
it is mandatory to use the ::ccl_err_clear() function to free the error object
and reset its value to `NULL`. Not doing so is a bug, especially if more
error-reporting functions are to be called moving forward. For example:

~~~~~~~~~~~~~~~{.c}
CCLContext* ctx;
CCLProgram* prg;
CCLError* err = NULL;
~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~{.c}
prg = ccl_program_new_from_source_file(ctx, "program.cl", &err);
if (err) {
    /* Print the error message, but don't terminate program. */
    fprintf(stderr, "%s", err->message);
    ccl_err_clear(&err);
}
~~~~~~~~~~~~~~~

Even if the program terminates due to an error, the ::ccl_err_clear() function
can be still be called to destroy the error object, avoiding memory leaks to be
reported by tools such as [Valgrind](http://valgrind.org/).

The internals of ::CCLErr-based error handling are discussed in further detail
in section @ref ug_deps "The GLib and OpenCL dependencies".

## Wrapper modules {#ug_wrappers}

Each [OpenCL class](http://www.khronos.org/registry/cl/sdk/2.1/docs/man/xhtml/classDiagram.html)
is associated with a _cf4ocl_ [module](modules.html). At the most basic level,
each module offers a wrapper class and functions which wrap or map their
OpenCL equivalents. This, in itself, already simplifies working with OpenCL in
C, because the _cf4ocl_ wrapper classes internally manage related objects, which
may be created during the course of a program. As such, client code just needs
to follow the @ref ug_new_destroy "new/destroy" rule for directly created
objects, thus not having to worry with memory allocation/deallocation of
intermediate objects.

In most cases, however, each _cf4ocl_ module also provides methods for
other common and not-so-common OpenCL host code patterns, allowing the
programmer to avoid their verbosity and focus on OpenCL device code.

All _cf4ocl_ wrapper classes extend the ::CCLWrapper* abstract
wrapper class. The properties and methods of this class, which are
concerned with reference counts, wrapping/unwrapping of OpenCL objects
and getting object information, are essentially of internal use by other
_cf4ocl_ classes. This functionality is also available to client code
which requires a more advanced integration with _cf4ocl_, as explained
in the @ref ug_cclwrapper "advanced" section.

Several OpenCL objects, namely `cl_platform_id`, `cl_context` and
`cl_program`, have a direct relationship with a set of `cl_device_id`
objects. In order to map this relationship, _cf4ocl_ provides the
::CCLDevContainer* class, which is an intermediate class between the
::CCLWrapper* parent class and the ::CCLPlatform*, ::CCLContext* and
::CCLProgram* wrappers. The ::CCLDevContainer* class implements
functionality for managing a set of ::CCLDevice* wrapper instances.
This functionality is exposed to client code through concrete wrapper
methods. For example, the ::CCLContext* class provides the
::ccl_context_get_all_devices(), ::ccl_context_get_device() and
::ccl_context_get_num_devices() methods for this purpose.

### Platform module {#ug_platform}

@copydoc CCL_PLATFORM_WRAPPER

### Device module {#ug_device}

@copydoc CCL_DEVICE_WRAPPER

### Context module {#ug_context}

@copydoc CCL_CONTEXT_WRAPPER

### Command queue module {#ug_queue}

@copydoc CCL_QUEUE_WRAPPER

### Memory object module {#ug_memobj}

@copydoc CCL_MEMOBJ_WRAPPER

### Buffer module {#ug_buffer}

@copydoc CCL_BUFFER_WRAPPER

### Image module {#ug_image}

@copydoc CCL_IMAGE_WRAPPER

### Sampler module {#ug_sampler}

@copydoc CCL_SAMPLER_WRAPPER

### Program module {#ug_program}

@copydoc CCL_PROGRAM_WRAPPER

### Kernel module {#ug_kernel}

@copydoc CCL_KERNEL_WRAPPER

#### Kernel arguments module {#ug_kernel_args}

@copydoc CCL_KERNEL_ARG

### Event module {#ug_event}

@copydoc CCL_EVENT_WRAPPER

#### Event wait lists module {#ug_event_wait_lists}

@copydoc CCL_EVENT_WAIT_LIST

## Other modules {#ug_othermodules}

### Device selector module {#ug_devsel}

@copydoc CCL_DEVICE_SELECTOR

### Device query module {#ug_devquery}

@copydoc CCL_DEVICE_QUERY

### Errors module {#ug_errors}

@copydoc CCL_ERRORS

### Platforms module {#ug_platforms}

@copydoc CCL_PLATFORMS

### Profiler module {#ug_profiling}

@copydoc CCL_PROFILER

# Bundled utilities {#ug_utils}

_cf4ocl_ is bundled with the following utilities:

* @ref ccl_devinfo "ccl_devinfo" - @copybrief ccl_devinfo
* @ref ccl_c "ccl_c" - @copybrief ccl_c
* @ref ccl_plot_events "ccl_plot_events" - @copybrief ccl_plot_events.py

# Advanced {#ug_advanced}

## Wrapper architecture {#ug_architecture}

The wrapper classes, which wrap OpenCL types, are implemented using an
object-oriented (OO) approach in C. While C does not directly provide
OO constructs, it is possible to implement features such as inheritance,
polymorphism or encapsulation. Using this approach, _cf4ocl_ is able to
offer a clean and logical class system, while being available in a form
(C) which can be directly or indirectly invoked from other programming
languages.

Each _cf4ocl_ wrapper class is defined by a source (.c) file and a
header (.h) file. The former contains the private class properties and
the method implementations, while the later defines its public API. The
class body is implemented in the source file as a C `struct`; the header
file provides an opaque pointer to it, which is the public side of the
class from a client code perspective. Inheritance is implemented by
including a member representing the parent class `struct` in the body of
the child class `struct`. This requires the sharing of parent class
implementations. In order to keep these opaque, the respective `struct`
is defined in "private" header files which are not included in the
public API. This way, instances of the child class can be cast to its
parent type when required. The child class `struct` effectively extends
the parent class `struct`. An example of this approach can be shown with
the definitions of the abstract ::CCLWrapper* class and of the concrete
::CCLEvent* class, which extends ::CCLWrapper*:

<em>In @c %_ccl_abstract_wrapper.h (not part of public API):</em>
@code{.c}
/* Base class for all OpenCL wrappers. */
struct ccl_wrapper {

    /* The class or type of wrapped OpenCL object. */
    CCLClass class;

    /* The wrapped OpenCL object. */
    void* cl_object;

    /* Information about the wrapped OpenCL object. */
    CCLWrapperInfoTable* info;

    /* Reference count. */
    int ref_count;

};
@endcode

_In ccl_common.h:_

@code{.c}
/* Event wrapper class type declaration. */
typedef struct ccl_event CCLEvent;
@endcode

_In ccl_event_wrapper.c:_

@code{.c}
/* Event wrapper class, extends CCLWrapper */
struct ccl_event {

    /* Parent wrapper object. */
    CCLWrapper base;

    /* Event name, for profiling purposes only. */
    const char* name;

};
@endcode

Methods are implemented as functions which accept the object on which
they operate as the first parameter. When useful, function-like macros
are also used as class methods, such as the case of the
@ref ug_getinfo "info macros". Polymorphism is not used, as the so
called "abstract" methods are just functions which provide common
operations to concrete methods, named differently for each concrete
class. For example, the @c ccl_dev_container_get_device() abstract
method is called by the ::ccl_context_get_device(),
::ccl_platform_get_device() and ::ccl_program_get_device() concrete
methods, for which it provides common functionality.

The _cf4ocl_ class hierarchy is shown in the following inheritance
diagram:

@dot
digraph cf4ocl {
    rankdir=RL;
    node [shape=record, fontname=Helvetica, fontsize=10];
    wrapper [ label="CCLWrapper*" URL="@ref ccl_wrapper"];
    devcon [ label="CCLDevContainer*" URL="@ref ccl_dev_container"];
    ctx [ label="CCLContext*" URL="@ref ccl_context"];
    prg [ label="CCLProgram*" URL="@ref ccl_program"];
    platf [ label="CCLPlatform*" URL="@ref ccl_platform"];
    memobj [ label="CCLMemObj*" URL="@ref ccl_memobj"];
    buf [ label="CCLBuffer*" URL="@ref ccl_buffer"];
    img [ label="CCLImage*" URL="@ref ccl_image"];
    dev [ label="CCLDevice*" URL="@ref ccl_device"];
    evt [ label="CCLEvent*" URL="@ref ccl_event"];
    krnl [ label="CCLKernel*" URL="@ref ccl_kernel"];
    queue [ label="CCLQueue*" URL="@ref ccl_queue"];
    smplr [ label="CCLSampler*" URL="@ref ccl_sampler"];
    devcon -> wrapper;
    ctx -> devcon;
    prg -> devcon;
    platf -> devcon;
    memobj->wrapper;
    buf -> memobj;
    img -> memobj;
    dev -> wrapper;
    evt -> wrapper;
    krnl -> wrapper;
    queue -> wrapper;
    smplr -> wrapper;
}
@enddot

### The CCLWrapper base class {#ug_cclwrapper}

The ::CCLWrapper* base class is responsible for common functionality of wrapper
objects, namely:

* Wrapping/unwrapping of OpenCL objects and maintaining a one-to-one
relationship between wrapped objects and wrapper objects
* Low-level memory management (allocation and deallocation)
* Reference counting
* Information handling (i.e., handling of data returned by the several
`clGet*Info()` OpenCL functions)

Wrapper constructors create the OpenCL object to be wrapped, but
delegate memory allocation to the special `ccl_<class>_new_wrap()`
functions. These accept the OpenCL object, and in turn call the
@c ccl_wrapper_new() function, passing it not only the object, but also
the size in bytes of the wrapper to be created. The
@c ccl_wrapper_new() function allocates memory for the wrapper
(initializing this memory to zero), and keeps the OpenCL object
(wrapping it) in the created wrapper instance. For example, ::ccl_kernel_new()
creates the `cl_kernel` object with the clCreateKernel() OpenCL function, but
then relies on the ::ccl_kernel_new_wrap() function (and thus, on
@c ccl_wrapper_new()) for allocation and initialization of the new ::CCLKernel*
wrapper object memory.

The destruction of wrapper objects and respective memory deallocation
is performed in a similar fashion. Each wrapper class has its own
`ccl_<class>_destroy()` method, but this method delegates actual
object release to the "abstract" @c ccl_wrapper_unref() function. This
function accepts the wrapper to be destroyed, its size in bytes, and
two function pointers: the first, with prototype defined by
@c ccl_wrapper_release_fields(), is a wrapper specific function for
releasing internal wrapper objects, which the super class has no
knowledge of; the second is the OpenCL object destructor function, with
prototype defined by @c ccl_wrapper_release_cl_object(). Continuing on
the kernel example, the ::ccl_kernel_destroy() method delegates kernel
wrapper destruction to @c ccl_wrapper_unref(), passing it the kernel
wrapper object, its size (i.e. `sizeof(` ::CCLKernel `)`), the "private"
(static in C) @c ccl_kernel_release_fields() function for destroying
kernel internal objects, and the clReleaseKernel() OpenCL kernel
destructor function.

As such, all _cf4ocl_ wrapper objects use a common memory allocation and
deallocation strategy, implemented in the ::CCLWrapper* super class.

The `ccl_<class>_new_wrap()` special constructors respect the
@ref ug_new_destroy "new/destroy" rule. Wrappers created with their
special constructor must be released with the respective
`ccl_<class>_destroy()` function. This allows client code to create
OpenCL objects directly with OpenCL functions, and then wrap the objects
to take advantage of _cf4ocl_ functionality and features. The OpenCL
object can be retrieved from its wrapper at all times with the
respective `ccl_<class>_unwrap()` method.

If `ccl_<class>_new_wrap()` functions are passed an OpenCL object which
is already wrapped, a new wrapper will not be created. Instead, the
existing wrapper is returned, with its reference count increased by 1.
Thus, there is always a one-to-one relationship between wrapped OpenCL
objects and their respective wrappers. In reality, the
`ccl_<class>_destroy()` functions decreases the reference count of the
respective wrapper, only destroying it if the reference count reaches
zero. Client code can increase and decrease the reference count of a
wrapper object using the associated `ccl_<class>_ref()` and
`ccl_<class>_unref()` macros. The `ccl_<class>_ref()` macros call the
::ccl_wrapper_ref() function, casting the wrapper to its base class
(::CCLWrapper*), while the `ccl_<class>_unref()` macros are just aliases
for the respective `ccl_<class>_destroy()` functions.

The ::CCLWrapper* class maintains a static hash table which associates
OpenCL objects (keys) to _cf4ocl_ wrappers (values). Access to this
table is thread-safe and performed by the @c ccl_wrapper_new() and
@c ccl_wrapper_unref() functions.

The management of OpenCL object information is also handled by the
::CCLWrapper* class. The ::ccl_wrapper_get_info() method accepts
two wrapper objects, the first being the object to query; the second is
an auxiliary object required by some lower-level OpenCL info functions,
such clGetKernelWorkGroupInfo(), which requires a device object besides
the kernel object. ::ccl_wrapper_get_info() also requires the specification of
the type of query to perform via the ::CCLInfo enumeration.
::ccl_wrapper_get_info() returns a ::CCLWrapperInfo* object, which contains two
public properties: the queried value and its size. To be useful, the value must
be cast to the correct type. The ::ccl_wrapper_get_info_value() and
::ccl_wrapper_get_info_size() methods call ::ccl_wrapper_get_info(), but
directly return the value and size of the ::CCLWrapper* object, respectively.

The requested information is kept in the information table of the
respective wrapper object. When the wrapper object is destroyed, all the
information objects are also released. As such, client code does not
need to worry about freeing objects returned by the
`ccl_wrapper_get_info*()` methods. These also accept a `use_cache`
boolean argument, which if true, causes the methods to first search
for the information in the wrappers information table, in case it has
already been requested; if not, they proceed with the query as normal.

Client code will commonly use the @ref ug_getinfo "info macros" of each
wrapper in order to fetch information about the underlying OpenCL
object. These macros expand into the `ccl_wrapper_get_info*()` methods,
automatically casting objects and values to the appropriate type,
selecting the correct `clGet*Info()` function for the object being
queried. The cache is never used by the  @ref ug_getinfo "info macros".

### The CCLDevContainer class {#ug_ccldevcontainer}

The intermediate ::CCLDevContainer* class provides functionality for
managing a set of ::CCLDevice* wrapper instances, abstracting code
common to the ::CCLPlatform*, ::CCLContext* and ::CCLProgram* classes,
all of which internally keep a set of devices. The ::CCLDevContainer*
class contains three "abstract" methods for accessing the associated set
of ::CCLDevice* wrappers, namely:

* @c ccl_dev_container_get_all_devices() : get all ::CCLDevice* wrappers
in device container object.
* @c ccl_dev_container_get_device() : get ::CCLDevice* wrapper at given
index.
* @c ccl_dev_container_get_num_devices() : return number of devices in
device container object.

Concrete wrapper implementations rely on this functionality, which is exposed to
client code via specific methods, e.g. in the case of ::CCLProgram* objects,
these are ::ccl_program_get_all_devices(), ::ccl_program_get_device() and
::ccl_program_get_num_devices(), respectively.

### The CCLMemObj class {#ug_cclmemobj}

The relationship between the ::CCLMemObj* class and the ::CCLBuffer* and
::CCLImage* classes follows that of the respective
[OpenCL types](http://www.khronos.org/registry/cl/sdk/2.1/docs/man/xhtml/classDiagram.html).
In other words, both OpenCL images and buffers are memory objects with
common functionality, and _cf4ocl_ directly maps this relationship with
the respective wrappers.

## The GLib and OpenCL dependencies {#ug_deps}

_cf4ocl_ relies heavily on its two dependencies:
[GLib](https://developer.gnome.org/glib/stable/) and
[OpenCL](https://www.khronos.org/opencl/). In previous versions of _cf4ocl_
no special care was taken to shield client code from these dependencies, and in
some cases client applications were required to link against them during the
build process. However, developers may not wish to tackle additional libraries,
and keeping _cf4ocl_ self-contained promotes a cleaner build process. As such,
since version 2.1.0, client applications are not required to link against either
GLib or OpenCL, except if they specifically wish to use the functionality
provided by these libraries.

One feature of GLib previously exposed to _cf4ocl_ client code was its
[error reporting framework](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html),
since _cf4ocl_ uses it for internal and external error handling. From version
2.1.0 onwards, this framework is no longer directly exposed to client code. Two
backward-compatible changes were implemented:

* ::CCLErr directly replaces
  [GError](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#GError)
  (both represent the same underlying structure).
* ::ccl_err_clear() directly replaces (i.e., wraps)
  [g_clear_error()](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#g-clear-error).

Newly written client code should use the ::CCLErr class and the
::ccl_err_clear() function in order to avoid a direct dependency on GLib.

A second aspect of GLib indirectly exposed to client code is the use of its
[pointer arrays](https://developer.gnome.org/glib/stable/glib-Pointer-Arrays.html)
as the underlying type for the ::CCLDevSelDevices, ::CCLDevSelFilters and
::CCLEventWaitList classes. The last two are automatically freed in typical
client code usage, but there can be situations in which ::CCLDevSelDevices
objects may have to be explicitly released. This can be accomplished with the
::ccl_devsel_devices_destroy() function, which is a wrapper for GLib's
[g_ptr_array_free()](https://developer.gnome.org/glib/stable/glib-Pointer-Arrays.html#g-ptr-array-free)
function. Thus, client code never needs to directly or explicitly manage GLib
[pointer arrays](https://developer.gnome.org/glib/stable/glib-Pointer-Arrays.html).

Finally, several _cf4ocl_ functions, such as ::ccl_devsel_get_device_strings(),
return a `NULL`-terminated array of strings. Previously, these arrays were
released using the
[g_strfreev()](https://developer.gnome.org/glib/stable/glib-String-Utility-Functions.html#g-strfreev)
GLib function. From version 2.1.0 onwards,  _cf4ocl_ provides the
::ccl_strv_clear() for this purpose. In practice this function is just a wrapper
for [g_strfreev()](https://developer.gnome.org/glib/stable/glib-String-Utility-Functions.html#g-strfreev).
Thus, this change does not break compatibility with existing client code.

## Log messages {#ug_log}

_cf4ocl_ internally uses the
[GLib message logging framework](https://developer.gnome.org/glib/stable/glib-Message-Logging.html)
to log messages and warnings. _cf4ocl_ log output is handled by
[GLib's default log handler](https://developer.gnome.org/glib/stable/glib-Message-Logging.html#g-log-default-handler),
which outputs warnings and messages to `stderr`. Debug messages are also shown
if the `G_MESSAGES_DEBUG` environment variable is set to `cf4ocl2`. If client
code wishes to redirect this output, it can do so by specifying another
[log function](https://developer.gnome.org/glib/stable/glib-Message-Logging.html#GLogFunc)
for the `cf4ocl2` log domain with
[g_log_set_handler()](https://developer.gnome.org/glib/stable/glib-Message-Logging.html#g-log-set-handler).
For example:

@code{.c}
/* Log function which outputs messages to a stream specified in user_data. */
void my_log_function(const gchar *log_domain, GLogLevelFlags log_level,
    const gchar *message, gpointer user_data) {

    g_fprintf((FILE*) user_data, "[%s](%d)>%s\n",
        log_domain, log_level, message);

}
@endcode
@code{.c}
FILE* my_file;
@endcode
@code{.c}
/* Add log handler for all messages from cf4ocl. */
g_log_set_handler("cf4ocl2", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
    my_log_function, my_file);
@endcode

This requires the client application to be linked against GLib..

@example ca.c
@example ca.cl
@example canon.c
@example canon.cl
@example list_devices.c
@example device_filter.c
@example image_fill.c
@example image_filter.c
@example image_filter.cl
