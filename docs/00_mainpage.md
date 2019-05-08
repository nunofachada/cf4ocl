@brief _cf4ocl_, a C Framework for OpenCL

Introduction {#intro}
============

## Overview {#intro_overview}

The C Framework for OpenCL, _cf4ocl_, is a cross-platform pure C object-oriented
framework for developing and benchmarking [OpenCL][] projects. It aims to:

1. Promote the rapid development of OpenCL programs in C, with support for C++.
2. Assist in the benchmarking of OpenCL events, such as kernel execution and
data transfers.
3. Simplify the analysis of the OpenCL environment and of kernel requirements.

## Features {#intro_features}

* Object-oriented interface to the OpenCL API
  * New/destroy functions, no direct memory alloc/free
  * Easy (and extensible) device selection
  * Simple event dependency mechanism
  * User-friendly error management
* OpenCL version and platform independent
* Integrated profiling
* Advanced device query utility
* Offline kernel compiler and linker

## Documentation {#intro_docs}

* @ref ug "User guide": @copybrief ug
* @ref tut "Tutorial": @copybrief tut
* @ref utils "Utilities": @copybrief utils
* @ref funlist "Function list": @copybrief funlist
* @htmlonly <a href="modules.html">Library modules: </a>@endhtmlonly
  Detailed description of each _cf4ocl_ module.
* @htmlonly <a href="examples.html">Examples: </a>@endhtmlonly
  A selection of examples implemented with _cf4ocl_.
* [Wiki][]: Useful information about _cf4ocl_.

## Small example

The example below shows a clean and fast way to create an OpenCL context with a
user-selected device:

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

## Not yet integrated

A few OpenCL API calls, most of which introduced with OpenCL 2.1, are
[not yet integrated][enhancements] with _cf4ocl_. However, this functionality is
still available to client code, because _cf4ocl_ can be used simultaneously with
raw OpenCL objects and functions.

## License

Library code is licensed under [LGPLv3][], while the remaining code (utilities,
examples and tests) is licensed under [GPLv3][].

## Other useful C/C++ frameworks/utilities for OpenCL

If _cf4ocl_ does not meet your requirements, take a look at the following
projects:

* [Simple OpenCL][]
* [The OpenCL utility library][]
* [OCL-MLA][]
* [oclkit][]
* [hiCL][]
* [ocl-ke][]

[OpenCL]: http://www.khronos.org/opencl/ "OpenCL"
[LGPLv3]: http://www.gnu.org/licenses/lgpl.html "LGPLv3"
[GPLv3]: http://www.gnu.org/licenses/gpl.html "GPLv3"
[Wiki]: https://github.com/fakenmc/cf4ocl/wiki
[enhancements]: https://github.com/fakenmc/cf4ocl/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement
[Simple OpenCL]: https://github.com/morousg/simple-opencl "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"
[oclkit]: https://github.com/matze/oclkit "oclkit"
[hiCL]: https://github.com/issamsaid/hiCL "high level OpenCL abstraction layer for scientific computing"
[ocl-ke]: https://github.com/anyc/ocl-ke "OpenCL kernel extractor"
