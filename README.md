cf4ocl - C Framework for OpenCL
===============================

About
-----

The C Framework for OpenCL (cf4ocl) is a pure C99 set of libraries and
utilities for speeding-up the development and benchmarking of OpenCL 
programs. It's main target platform is Linux, although previous
versions were sucessfully compiled in Windows with [MinGW](http://www.mingw.org/). 
The only dependency is [GLib](https://developer.gnome.org/glib/) (v2.23
or newer), the development files of which can be easily installed from
your distro's package manager. Library code is licensed under [LGPLv3](http://www.gnu.org/licenses/lgpl.html),
while the remaining code is licensed under [GPLv3](http://www.gnu.org/licenses/gpl.html).

The code is fully commented (docs can be generated with Doxygen, 
preferably with version 1.8.3 or newer), is compiled with `-Wall` and
`-Wextra` flags, and is veryfied with [cppcheck](http://cppcheck.sourceforge.net/).

Organization
------------

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

* Create/initialize OpenCL objects such as plaform, device, context, 
command queues, devices, and so on with just one instruction (also one
instruction for freeing/releasing these objects).
* Build an OpenCL program with one instruction by passing an array of
filenames containing the kernels.
* Automatic or manual device selecion.
* Doesn't hide OpenCL objects from you.

### CL Profiler

* Get detailed profiling information about OpenCL events, including
relative and absolute times.
* Supports multiple command queues and overlapping events (e.g. such
as simultaneous kernel execution and data transfer).

### GError framework

* Generic C error handling framework based on GError object from GLib.
* Just two macros.

Utilities
---------

### Device query

Simple implementation of a program for querying available OpenCL
platforms and devices with clean and useful output.

### Kernel info

Static analysis of OpenCL kernels.

How to use the library
----------------------

Take a look at the examples, utilities and tests to see how the 
library can be used.

Other useful C frameworks/utilities for OpenCL
----------------------------------------------

* [Simple OpenCL](http://code.google.com/p/simple-opencl/)
* [The OpenCL utility library](https://github.com/Oblomov/CLU)
* [Computing Language Utility](https://github.com/Computing-Language-Utility/CLU)
