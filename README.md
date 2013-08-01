Summary
=======

The C Framework for OpenCL (cf4ocl) is a pure C99 set of libraries and
utilities with the goal of facilitating: 1) the rapid development of 
OpenCL C programs; 2) the benchmark of OpenCL events, such as kernel 
execution and data transfers; and, 3) analysis of the OpenCL 
environment and kernel requirements.

cf4ocl is composed of four parts: i) library; ii) command-line 
utilities; iii) unit tests; and, iv) examples. The library offers 
functions which aim to achieve goals 1) and 2). The command-line 
utilities are focused on goal 3). The unit tests aim to verify that the 
library functions are working properly. Finally, the examples show how 
to integrate the library functions in fully working OpenCL programs.

The target platform of cf4ocl is Linux, although, being pure C99, it 
can be compiled to other platforms as long as the dependencies, 
[GLib][] and [OpenCL][], are met. The reference compiler is GCC with 
`-Wall` and `-Wextra` flags activated. The code is verified with 
[cppcheck][] and is fully commented. API documentation can be generated 
with [Doxygen][].

Library code is licensed under [LGPLv3][], while the remaining code is 
licensed under [GPLv3](http://www.gnu.org/licenses/gpl.html).

<!--- 
Table of contents
=================
[TOC]
-->

Introduction
============

Structure
---------

* **Library**
    * CL Utils
    * CL Profiler
    * GError framework
* **Library tests**
    * CL Profiler
    * GError framework
* **Utilities**
    * Device query
    * Kernel info
* **Examples**
    * Bank conflicts test
    * Matrix multiplication

Library
-------

### CL Utils

* Create/initialize OpenCL objects such as platform, device, context, 
command queues, devices, and so on with just one instruction (also one
instruction for freeing/releasing these objects).
* Build an OpenCL program with one instruction by passing an array of
filenames containing the kernels.
* Automatic or manual device selection.
* Doesn't hide OpenCL objects from you.

### CL Profiler

* Get detailed profiling information about OpenCL events, including
relative and absolute times.
* Supports multiple command queues and overlapping events (e.g. such
as simultaneous kernel execution and data transfer).

### GError framework

* Generic C error handling framework based on GError object from GLib.
* Just two macros.
* Required by CL Utils and CL Profiler.

Utilities
---------

### Device query

Simple implementation of a program for querying available OpenCL
platforms and devices with clean and useful output.

### Kernel info

Static analysis of OpenCL kernels.

How to use cf4ocl
=================

Dependencies
------------

In order to compile cf4ocl and use it in projects, two dependencies are
required: 1) [OpenCL][]; and, 2) [GLib][] (v2.32 or newer).

### OpenCL

The OpenCL headers (for compiling) and shared library (for linking) can
be installed from the packages offered by different OpenCL vendors such
as [AMD][AMDSDK], [Intel][IntelSDK] or [Nvidia][NvidiaSDK]. 

Alternatively, it is possible to install an OpenCL development 
environment in a some recent Linux distributions. For example, in Debian
7 and Ubuntu 13.04:

    $ sudo apt-get install ocl-icd-opencl-dev

This will install the headers and shared library system-wide, avoiding 
the need to specify their location to GCC. However, to actually run an 
OpenCL program it is necessary to have a vendor implementation 
installed.

### GLib

The simplest way to install [GLib][] is using the package manager of
the Linux distribution. For Debian and Ubuntu:

    $ sudo apt-get install libglib2.0-dev
    
It is also advisable to install the `libglib2.0-0-dbg` or equivalent
package to allow debugging at the GLib level.

Library, utilities, tests and examples
--------------------------------------

Assuming a working development environment with OpenCL and GLib 
properly setup, to build everything it is simply necessary to run `make`
within the cf4ocl directory.

It is possible to build only specific targets, for example:

    $ make library
    $ make utils
    $ make tests
    $ make examples

If the build failed, confirm if OpenCL and GLib are properly installed,
and if GCC can find the OpenCL headers and shared library. If not,
set the headers location in the `CLINCLUDES` Makefile variable, and set
the shared library location in the `CLLIBDIR` Makefile variable.

Using the library in a new project
----------------------------------

Any of the three elements of the library can be used in a workflow
without the need to use the others. However, both CL Utils and CL 
Profiler internally use the GError framework.

### Compiling and linking

When building a program based on cf4ocl (or any OpenCL program) with 
GCC, it may be necessary to specify the location of the OpenCL headers 
with the `-I` flag, and the location of the shared library 
(libOpenCL.so) with the `-L` flag. If the OpenCL development files were
installed from the package manager (e.g. the `ocl-icd-opencl-dev` 
package in Debian and Ubuntu), this is usually not required.

Additionally, in order to **compile** a program with GLib (required by 
cf4ocl), the following line should be added to the GCC syntax:

    `pkg-config --cflags glib-2.0`
    
In order to **link** a program with GLib, it is necessary to add the 
following line to the GCC syntax:

    `pkg-config --libs glib-2.0`

These two lines will expand correctly to the `-I` and `-l` flags 
required for compiling and linking, respectively.

### Using CL Utils

TO DO

### Using CL Profiler

The goal of CL Profiler is to provide detailed benchmarking information 
about OpenCL events such as kernel execution, data transfers, and so on.
CL Profiler is prepared to handle overlapping events, which usually
take place when the programmer is trying to optimize its application
by simultaneously transfer data to and from the OpenCL device and 
execute kernels, using different command queues.

CL Profiler consists of two files, `clprofiler.c` and `clprofiler.h`.
In order to use CL Profiler in a project, it is necessary to include
the `clprofiler.h` header file.

For the purpose of this explanation, we will consider that two command 
queues are being used:

```c
cl_command_queue queue0; /* Used for host-device data transfer. */
cl_command_queue queue1; /* Used for kernel execution. */
```

Additionally, we will consider the following OpenCL events:

```c
cl_event ev_transf_in;  /* Transfer data from host to device. */
cl_event ev_kernel_A_1; /* Execute kernel A on device. */
cl_event ev_kernel_B;   /* Execute kernel B on device. */
cl_event ev_kernel_A_2; /* Execute kernel A on device again. */
cl_event ev_transf_out; /* Transfer data from device to host. */
```

The `ProfCLProfile` structure forms the basis of CL Profiler. It can:
1) measure the total elapsed time of the application (or the relevant 
part of the application); and, 2) keep track of the device time required
by the OpenCL events. The following instruction creates a new 
`ProfCLProfile` structure:

```c
/* Create a new ProfCLProfile structure. */
ProfCLProfile* profile = profcl_profile_new();
```

In order to start measuring the relevant part of the application, the
following instruction should be issued:

```c
/* Start basic timming / profiling. */
profcl_profile_start(profile);
```

At this time, the typical OpenCL application workflow, such as 
transferring data and executing kernels, should take place. The above 
defined events must be associated with the respective `clEnqueue*` 
OpenCL functions in order to be later analyzed. A typical workflow 
may be finalized with the following instructions:

```c
/* Finish all pending OpenCL operations. */
clFinish(queue0);
clFinish(queue1);
```

Profiling should be stopped at this point.

```c
profcl_profile_stop(profile); 
```

Now the events can be added to the profiler structure. The
`profcl_profile_add` function uses the second parameter (a string)
as a key to differentiate between events. Thus, if the same key
is given for different OpenCL events, CL Profiler will consider 
it to be the same _semantic_ event. This can be useful for aggregating
execution times of events which occur innumerous times in a cyclic 
fashion (in a _for_ loop, for example). 

```c
/* Add events to be profiled/analyzed. */
profcl_profile_add(profile, "Transfer data to device", ev_transf_in, NULL);
profcl_profile_add(profile, "Kernel A", ev_kernel_A_1, NULL);
profcl_profile_add(profile, "Kernel B", ev_kernel_B, NULL);
profcl_profile_add(profile, "Kernel A", ev_kernel_A_2, NULL);
profcl_profile_add(profile, "Transfer data from device", ev_transf_out, NULL);
```

The above code will consider OpenCL events `ev_kernel_A_1` and `ev_kernel_A_2`
to be the same semantic event because the same key, string _Kernel A_, is
used. Thus, the total execution time for the semantic event _Kernel A_ will 
be the sum of respective two OpenCL events. CL Profiler can even
determine overlaps of a semantic event with itself (i.e., two overlapping
OpenCL events which are added for analysis with the same key).

After all the events are added, it is necessary to instruct CL 
Profiler to perform the required calculations in order to determine
the absolute and relative times of all events, and how these correlate
with the total elapsed time of the relevant part of the application.

```c
profcl_profile_aggregate(profile, NULL);
```

Finally, the complete benchmarking info can be printed:

```c
profcl_print_info(profile, PROFCL_AGGEVDATA_SORT_TIME, NULL);
```

Two detailed tables will be printed: one for individual events (sorted by 
name or execution time), and another showing event overlaps, if any occurred.

Only at this time should the OpenCL events be freed.

```c
clReleaseEvent(ev_transf_in);
clReleaseEvent(ev_kernel_A_1);
clReleaseEvent(ev_kernel_B);
clReleaseEvent(ev_kernel_A_2);
clReleaseEvent(ev_transf_out);
```

The last parameter of some of the `profcl_*` functions is used for error 
handling and to obtain detailed error messages if an error occurs. By 
passing `NULL`, the programmer choses to ignore that feature. Such approach 
is not critical because all of the error-prone `profcl_*` functions also return
their execution status code. No error handling is performed in this 
explanation, though.

For map events, CL Profiler provides the `profcl_profile_add_composite`
function, which accepts two OpenCL events, one relative to the _map_
operation, and the other to the _unmap_ operation. The function uses
the start instant of the _map_ event, and the _end_ instant of the
_unmap_ event, in order to build a composite semantic event.

### Using GError Framework (GErrorF)

The main purpose of GErrorF is to provide error handling constructs to 
CL Utils and CL Profiler. No knowledge of GErrorF is required to use
CL Utils and/or CL Profiler. However, because it is sufficiently generic 
to be used in any C application, a specific description is warranted.

GErrorF uses GLib's `GError` object for function error reporting. This
explanation assumes minimal knowledge of 
[GLib's error reporting][gliberror].

GErrorF is defined by two macros in `gerrorf.h`:

* `gef_if_error_create_goto` - Catches errors from non-GError aware
functions.
* `gef_if_error_goto` - Catches errors from GError aware functions.

GErrorF establishes an error handling methodology for C programs not
entirely dissimilar to the pattern used in Linux kernel development. 
Any function producing recoverable runtime errors, from `main` to 
functions located deeper in the call stack, can benefit from this 
approach. The general usage of GErrorF is as follows:

```c
include "gerrorf.h";

...

int main(int argc, char* argv[]) {

    ...

    /* Must initialize every allocable pointers and objects */
    /* to NULL.                                             */
    int some_vector* = NULL;

    /* GError object. */
    GError *err = NULL;

    /* Function return status. */
    int status = SUCCESS_CODE;

    ...

    /* Call a GError aware function. */
    some_function(params, &err);

    /* Catch possible error in GError-aware function. In this    */
    /* the GError object is initialized by the called function.  */
    gef_if_error_goto(
        err,              /* GError object. */
        SOME_ERROR_CODE,  /* Error code to set in status. */
        status,           /* Function return status. */
        error_handler     /* Label to goto in case of error. */
    );

    /* In the previous function it is possible to replace an app */
    /* specific error code with GErrorF special constants        */
    /* GEF_USE_STATUS and GEF_USE_GERROR. The former leaves the  */
    /* status variable untouched (useful for cases were the      */
    /* function itself returns a usable int status), while the   */
    /* later sets status to the error code set in the GError     */
    /* object. */

    ...

    /* Call a non-GError aware function. */
    some_vector = (int*) malloc(sizeof(int) * SOME_SIZE);

    /* Catch possible error in non-GError aware function. */
    gef_if_error_create_goto(
        err,                    /* GError object.                  */
        SOME_QUARK_ERROR,       /* GLib GQuark identifier.         */
        some_vector == NULL,    /* Error condition.                */
        SOME_ERROR_CODE,        /* Error code to set in err.       */
        error_handler,          /* Label to goto in case of error. */
        "Unable to alloc. mem." /* Error msg to set in err.        */
    );

    ...

    /* If we get here, there was no error, goto cleanup. */
    g_assert(err == NULL);  /* Make sure err is NULL. */
    goto cleanup;           /* Goto the cleanup section. */
	
error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert (err != NULL);
    /* Print error message. */
    fprintf(stderr, "Error message: %s\n", err->message);
    /* Make sure function status contains an error code. */
    if (status == SUCCESS_CODE) status = err->code; 
    /* Free the GError object. */
    g_error_free(err);

cleanup:	
    /* Free any allocated memory. */
    if (some_vector) free(some_vector);

    ...

    /* Return program status. */
    return status;
}

...

/* This function is GError-aware, and will initialize the GError */
/* object if an error occurs. The GError object usually comes    */
/* as the last parameter.                                        */
void some_function(some params, GError** err) {

    ...

    FILE* fp;
    const char* filename = "somefile.txt";

    ...

    /* Try to open a file. This function is not GError aware. */
    fp = fopen(filename, "r");

    /* Catch possible error in non-GError aware function. */
    gef_if_error_create_goto(
        *err,                     /* GError object.                  */
        SOME_QUARK_ERROR,         /* GLib GQuark identifier.         */
        fp == NULL,               /* Error condition.                */
        SOME_ERROR_CODE,          /* Error code to set in err.       */
        error_handler,            /* Label to goto in case of error. */
        "Unable to open file %s", /* Error msg to set in err.        */
        filename                  /* Extra args for error msg.       */
    );

    ...

    /* If we got here, everything is OK.                          */
    /* It's good practice to check if err is NULL (caller doesn't */
    /* care about error reporting OR if a non-null err is         */
    /* pointing to NULL (i.e. no error was reported).             */
    g_assert (err == NULL || *err == NULL);

    /* Goto finish label, ignoring the error handling section.    */
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so,   */
    /* i.e. either the caller doesn't care about error reporting, */
    /* in which case err is NULL, OR a non-null err is in fact    */
    /* pointing to an initialized GError object.                  */
    g_assert (err == NULL || *err != NULL);

    /* Run any other error handling code. */
    ...

finish:	

    /* Close the file, if open. */
    if (fp) fclose(fp);

    /* Perform additional required cleanup (free's and so on). */
    ...
    
    /* Bye. */
    return;

}

```

As can be observed, GErrorF enforces a strict programming pattern, 
which requires that complying functions follow a set of rules:

* Define all pointers and objects in the beginning of the function 
and initialize them to NULL.
* Define a pointer to a GError object and set it to NULL.
* Always test for errors after calls to error throwing functions
with `gef_if_error_create_goto` (non-GError aware functions) or
`gef_if_error_goto` (GError aware functions). If an error occurs
program execution will jump to the error handling block.
* Use the following pattern at the end of the function:
    * goto cleanup
    * error handling block
    * cleanup: perform clean up
    * return (possibly with a status code)

This pattern avoids many bugs and makes error catching and handling
possible in C. However it is not to everyone's taste, and is thus
a completely optional aspect of the cl4ocl framework.

Generating the API documentation
--------------------------------

The API documentation can be generated with `make` if [Doxygen][] 
(version 1.8 or newer) is installed. In the cf4ocl directory run:

    $ make docs
    
The documentation will be generated in the `./doc` folder. The direct
use of the `doxygen` command should be avoided because 
[Doxygen's markdown][doxymd] is not 100% compatible with the 
[GitHub Flavored Markdown][ghmd] used in this document. When building 
the documentation with `make`, the required conversion is performed 
first.

Other useful C frameworks/utilities for OpenCL
==============================================

If cf4ocl does not meet your requirements, take a look at the following
projects:

* [Simple OpenCL][]
* [The OpenCL utility library][]
* [Computing Language Utility][]
* [OCL-MLA][]

<!---
Conclusions
===========

References
==========

Appendices
==========
-->

[GLib]: https://developer.gnome.org/glib/ "GLib"
[OpenCL]: http://www.khronos.org/opencl/ "OpenCL"
[Doxygen]: http://www.doxygen.org/‎ "Doxygen"
[cppcheck]: http://cppcheck.sourceforge.net/ "cppcheck"
[LGPLv3]: http://www.gnu.org/licenses/lgpl.html) "LGPLv3"
[GPLv3]: http://www.gnu.org/licenses/gpl.html "GPLv3"
[AMDSDK]: http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/ "AMD"
[IntelSDK]: http://software.intel.com/en-us/vcsource/tools/opencl-sdk "Intel"
[NvidiaSDK]: https://developer.nvidia.com/category/zone/cuda-zone "Nvidia"
[clheaders]: http://www.khronos.org/registry/cl/ "Khronos"
[markdown]: http://daringfireball.net/projects/markdown/ "Markdown"
[doxymd]: http://www.stack.nl/~dimitri/doxygen/manual/markdown.html "Doxygen Markdown"
[ghmd]: https://help.github.com/articles/github-flavored-markdown "Github Flavored Markdown"
[gliberror]: https://developer.gnome.org/glib/2.32/glib-Error-Reporting.html "GLib Error Reporting"

[Simple OpenCL]: http://code.google.com/p/simple-opencl/ "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[Computing Language Utility]: https://github.com/Computing-Language-Utility/CLU "Computing Language Utility"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"
