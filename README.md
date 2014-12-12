Summary
=======

The C Framework for OpenCL, _cf4ocl_, is a cross-platform pure C
object-oriented framework for developing and benchmarking [OpenCL][]
projects in C. It aims to:

1. Promote the rapid development of OpenCL programs in C, with support
for C++.
2. Assist in the benchmarking of OpenCL events, such as kernel execution
and data transfers.
3. Simplify the analysis of the OpenCL environment and of kernel
requirements.

Features
========

* Object-oriented interface to the OpenCL API
  * New/destroy functions, no direct memory alloc/free
  * Easy (and extensible) device selection
  * Simple event dependency mechanism
  * User-friendly error management
* OpenCL version independent
* Integrated profiling

Documentation
=============

* [User guide and API](http://fakenmc.github.io/cf4ocl/docs/latest/)
* [Tutorial](http://fakenmc.github.io/cf4ocl/docs/latest/tut.html)
* [Wiki](https://github.com/FakenMC/cf4ocl/wiki)
* [Examples](http://fakenmc.github.io/cf4ocl/docs/latest/examples.html)

Feeback
=======

Get _cf4ocl_, either by [building from source](https://github.com/FakenMC/cf4ocl/wiki/Build%20and%20install%20from%20source)
or using the provided [binaries](https://github.com/FakenMC/cf4ocl/wiki/Install-the-binaries).
Take a look at the [examples](http://fakenmc.github.io/cf4ocl/docs/latest/examples.html)
and the [user guide and API](http://fakenmc.github.io/cf4ocl/docs/latest/).
Any feedback is welcome.

Not yet implemented
===================

* [OpenGL](https://github.com/FakenMC/cf4ocl/issues/3) and
[DirectX](https://github.com/FakenMC/cf4ocl/issues/4) interoperability
* [Support for sub-devices](https://github.com/FakenMC/cf4ocl/issues/6)
* [Pipes](https://github.com/FakenMC/cf4ocl/issues/8) and
[SVM](https://github.com/FakenMC/cf4ocl/issues/7) (OpenCL 2.0 only)

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

[OpenCL]: http://www.khronos.org/opencl/ "OpenCL"
[LGPLv3]: http://www.gnu.org/licenses/lgpl.html) "LGPLv3"
[GPLv3]: http://www.gnu.org/licenses/gpl.html "GPLv3"
[Simple OpenCL]: http://code.google.com/p/simple-opencl/ "Simple OpenCL"
[The OpenCL utility library]: https://github.com/Oblomov/CLU "The OpenCL utility library"
[Computing Language Utility]: https://github.com/Computing-Language-Utility/CLU "Computing Language Utility"
[OCL-MLA]: http://tuxfan.github.io/ocl-mla/ "OCL-MLA"

