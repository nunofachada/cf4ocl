/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 * File containing xor kernel for testing. This kernel delegates the xor to
 * another function which is declared in the header file.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "xor_impl.cl.h"

/**
 * Performs xor of two vectors and a constant.
 *
 * @param[in] a First vector to xor.
 * @param[in] b Second vector to xor.
 * @param[out] c Vector containing xor result.
 * @param[in] d Constant to xor.
 * */
__kernel void test_xor(
	__global const uint *a,
	__global const uint *b,
	__global uint *c, uint d) {

	/* Get global ID. */
	int gid = get_global_id(0);

	/* Perform sum. */
	c[gid] = do_xor(a[gid], b[gid], d);
}
