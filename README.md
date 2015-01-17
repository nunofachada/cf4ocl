News
====

*17 Jan. 2015* 

The 2.0.1 version fixes a specific issue with OpenCL <= 1.2 devices and
is available in the [releases](https://github.com/FakenMC/cf4ocl/releases)
page.

Summary
=======

The C Framework for OpenCL, _cf4ocl_, is a cross-platform pure C
object-oriented framework for developing and benchmarking [OpenCL][]
projects in C. It aims to:

1. Promote the rapid development of OpenCL host programs in C (with
support for C++) and avoid the tedious and error-prone boilerplate code
usually required.
2. Assist in the benchmarking of OpenCL events, such as kernel execution
and data transfers. Profiling comes for **free** with _cf4ocl_.
3. Simplify the analysis of the OpenCL environment and of kernel
requirements.
4. Allow for all levels of integration with existing OpenCL code: use as
much or as few of _cf4ocl_ required for your project, with full access
to the underlying OpenCL objects and functions at all times.

Features
========

* Object-oriented interface to the OpenCL API
  * New/destroy functions, no direct memory alloc/free
  * Easy (and extensible) device selection
  * Simple event dependency mechanism
  * User-friendly error management
* OpenCL version independent
* Integrated profiling
* Tested on Linux, OSX and Windows

Documentation
=============

* [User guide and API](http://fakenmc.github.io/cf4ocl/docs/latest/)
* [Tutorial](http://fakenmc.github.io/cf4ocl/docs/latest/tut.html)
* [Wiki](https://github.com/FakenMC/cf4ocl/wiki)
* [Examples](http://fakenmc.github.io/cf4ocl/docs/latest/examples.html)

Feedback and collaboration
==========================

Download or clone _cf4ocl_, [build and install it](https://github.com/FakenMC/cf4ocl/wiki/Build%20and%20install%20from%20source),
and [build](https://github.com/FakenMC/cf4ocl/wiki/Using-cf4ocl-in-a-new-project)
a small example, such as the one bellow, which shows a nice and fast way
to get an OpenCL context with a user-selected device:

```c
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
```

If you like this project and want to contribute, take a look at the
existing [issues](https://github.com/FakenMC/cf4ocl/issues). We also
need help with [binary packaging](https://github.com/FakenMC/cf4ocl/wiki/Install-the-binaries)
for different OSes. Other improvements or suggestions are of course,
welcome. We appreciate any feedback.

Not yet integrated
==================

The following aspects of OpenCL are not yet integrated with _cf4ocl_:

* [OpenGL](https://github.com/FakenMC/cf4ocl/issues/3) and
[DirectX](https://github.com/FakenMC/cf4ocl/issues/4) interoperability
* [Pipes](https://github.com/FakenMC/cf4ocl/issues/8) and
[SVM](https://github.com/FakenMC/cf4ocl/issues/7) (OpenCL 2.0 only)

This functionality is still available to client code, because _cf4ocl_
can be used simultaneously with raw OpenCL objects and functions.

License
=======

Library code is licensed under [LGPLv3][], while the remaining code
(utilities, examples and tests) is licensed under [GPLv3][].

Other useful C frameworks/utilities for OpenCL
==============================================

If _cf4ocl_ does not meet your requirements, take a look at the following
projects:

* [Simple OpenCL][]
* [The OpenCL utility library][]
* [Computing Language Utility][]
* [OCL-MLA][]
* [oclkit][]

[OpenCL]: http://www.khronos.org/opencl/ "OpenCL"
[LGPLv3]: http://www.gnu.org/licenses/lgpl.html "LGPLv3"
[GPLv3]: http://www.gnu.org/licenses/gpl.html "GPLv3"
[Simple OpenCL]: http://code.google.com/p/simple-opencl/ "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[Computing Language Utility]: https://github.com/Computing-Language-Utility/CLU "Computing Language Utility"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"
[oclkit]: https://github.com/matze/oclkit "oclkit"
