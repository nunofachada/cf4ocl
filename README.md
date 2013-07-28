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

_In progress_

Gives you detailed information about OpenCL events (kernel execution, 
data transfers, etc), including a table dedicated to overlapped 
execution of said events.

Basic example:

```c
ProfCLProfile* profile = profcl_profile_new();
cl_event events[NUMBER_OF_CL_EVENTS];
...

/* Start basic timming / profiling. */
profcl_profile_start(profile);

/* OpenCL data transfers, kernel executions, etc. */
...
clFinish(queue0);
clFinish(queue1);
...

/* Manage and show profiling info */
profcl_profile_stop(profile); 
profcl_profile_add(profile, "Transfer data to device", events[0], NULL);
profcl_profile_add(profile, "Kernel 1 execution", events[1], NULL);
profcl_profile_add(profile, "Kernel 2 execution", events[2], NULL);
...
profcl_profile_aggregate(profile, NULL);
profcl_print_info(profile, PROFCL_AGGEVDATA_SORT_TIME, NULL);

/* Two nice detailed tables will be printed: one for individual */
/* events (sorted by name or execution time), and another describing */
/* overlap of events. */
```

### Using GErrorF

TO DO

Generating the API documentation
--------------------------------

The API documentation can be generated with [Doxygen][] (version 1.8 or
newer). In the cf4ocl directory run:

    $ doxygen
    
The documentation will be generated in the `./doc` folder.

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

[Simple OpenCL]: http://code.google.com/p/simple-opencl/ "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[Computing Language Utility]: https://github.com/Computing-Language-Utility/CLU "Computing Language Utility"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"
