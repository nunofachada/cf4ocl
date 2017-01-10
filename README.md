[![Latest release](https://img.shields.io/github/release/fakenmc/cf4ocl.svg)](https://github.com/fakenmc/cf4ocl/releases)
[![LGPL Licence](https://img.shields.io/badge/license-LGPLv3-yellowgreen.svg)](https://opensource.org/licenses/LGPL-3.0/)
![Supported platforms](https://img.shields.io/badge/platform-windows%20%7C%20macos%20%7C%20linux%20%7C%20bsd-lightgrey.svg)

News
====

*4 July 2016*

Version 2.1.0 is available for download in the [releases][] page.

Summary
=======

The C Framework for OpenCL, _cf4ocl_, is a cross-platform pure C object-oriented
framework for developing and benchmarking [OpenCL][] projects. It aims to:

1. Promote the rapid development of OpenCL host programs in C (with support for
C++) and avoid the tedious and error-prone boilerplate code usually required.
2. Assist in the benchmarking of OpenCL events, such as kernel execution and
data transfers. Profiling comes for **free** with _cf4ocl_.
3. Simplify the analysis of the OpenCL environment and of kernel requirements.
4. Allow for all levels of integration with existing OpenCL code: use as much or
as few of _cf4ocl_ required for your project, with full access to the underlying
OpenCL objects and functions at all times.

Features
========

* Object-oriented interface to the OpenCL API
  * New/destroy functions, no direct memory alloc/free
  * Easy (and extensible) device selection
  * Simple event dependency mechanism
  * User-friendly error management
* OpenCL version and platform independent
* Integrated profiling
* Advanced device query utility
* Offline kernel compiler and linker

Documentation
=============

* [User guide and API](http://www.fakenmc.com/cf4ocl/docs/latest/)
* [Tutorial](http://www.fakenmc.com/cf4ocl/docs/latest/tut.html)
* [Wiki](https://github.com/fakenmc/cf4ocl/wiki)
* [Utilities](http://www.fakenmc.com/cf4ocl/docs/latest/utils.html)
* [Function list](http://www.fakenmc.com/cf4ocl/docs/latest/funlist.html)
* [Library modules](http://www.fakenmc.com/cf4ocl/docs/latest/modules.html)
* [Examples](http://www.fakenmc.com/cf4ocl/docs/latest/examples.html)

Feedback and collaboration
==========================

Download or clone _cf4ocl_,
[build and install it](https://github.com/fakenmc/cf4ocl/wiki/Build-and-install-from-source),
and [code](https://github.com/fakenmc/cf4ocl/wiki/Using-cf4ocl-in-a-new-project)
a small example, such as the one below, which shows a clean and fast way to
create an OpenCL context with a user-selected device:

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

If you like this project and want to contribute, take a look at the existing
[issues][]. We also need help with [binary packaging][] for different OSes.
Other improvements or suggestions are of course, welcome. We appreciate any
feedback.

Not yet integrated
==================

A few OpenCL API calls, most of which introduced with OpenCL 2.1, are
[not yet integrated][enhancements] with _cf4ocl_. However, this functionality is
still available to client code, because _cf4ocl_ can be used simultaneously with
raw OpenCL objects and functions.

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
* [OCL-MLA][]
* [oclkit][]
* [hiCL][]
* [ocl-ke][]

[releases]: https://github.com/fakenmc/cf4ocl/releases
[OpenCL]: http://www.khronos.org/opencl/ "OpenCL"
[LGPLv3]: http://www.gnu.org/licenses/lgpl.html "LGPLv3"
[GPLv3]: http://www.gnu.org/licenses/gpl.html "GPLv3"
[Wiki]: https://github.com/fakenmc/cf4ocl/wiki
[enhancements]: https://github.com/fakenmc/cf4ocl/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement
[issues]: https://github.com/fakenmc/cf4ocl/issues "issues"
[binary packaging]: https://github.com/fakenmc/cf4ocl/wiki/Install-the-binaries
[Simple OpenCL]: https://github.com/morousg/simple-opencl "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"
[oclkit]: https://github.com/matze/oclkit "oclkit"
[hiCL]: https://github.com/issamsaid/hiCL "high level OpenCL abstraction layer for scientific computing"
[ocl-ke]: https://github.com/anyc/ocl-ke "OpenCL kernel extractor"
