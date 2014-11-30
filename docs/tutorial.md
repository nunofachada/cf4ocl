Tutorial {#tut}
========

This tutorial is based on the `canon` example available in the
examples folder. The goal is to add two vectors, `a` and `b`, as well as
a constant `d`, and save the result in a third vector, 'c'. The OpenCL
kernel which performs this operation is given in the following code:

~~~~~~~~~~~~~~~{.c}
__kernel void sum(__global const uint *a, __global const uint *b,
	__global uint *c, uint d, uint buf_size) {

	/* Get global ID. */
	uint gid = get_global_id(0);

	/* Only perform sum if this workitem is within the size of the
	 * vector. */
	if (gid < buf_size)
		c[gid] = a[gid] + b[gid] + d;
}
~~~~~~~~~~~~~~~

For the purpose of this tutorial, we'll assume the kernel code is in a
file called `mysum.cl`, and the host code in `mysum.c`.

The _cf4ocl_ header should be included at the beggining:

~~~~~~~~~~~~~~~{.c}
#include <cf4ocl2.h>
~~~~~~~~~~~~~~~
