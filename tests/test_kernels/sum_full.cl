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
 * File containing sum kernel for testing.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

/**
 * Performs sum of two vectors plus a constant.
 *
 * @param[in] a First vector to sum.
 * @param[in] b Second vector to sum.
 * @param[out] c Vector containing sum.
 * @param[in] d Constant to sum.
 * */
__kernel void test_sum_full(
	__global const uint *a,
	__global const uint *b,
	__global uint *c, uint d) {

		/* Get global ID. */
		int gid = get_global_id(0);
		/* Perform sum. */
		c[gid] = a[gid] + b[gid] + d;
}
