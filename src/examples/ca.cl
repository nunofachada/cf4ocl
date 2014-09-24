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
 * File containing kernel for cellular automata simulation (Conway's
 * Game of Life).
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */


/* Number of neighbors of a CA cell. */
#define NUM_NEIGHS 8

/* List of neighbors of a CA cell. */
__constant int2 neighbors[] = {
	(int2) (-1,-1), (int2) (0,-1), (int2) (1,-1), (int2) (1,0),
	(int2) (1,1), (int2) (0,1), (int2) (-1,1), (int2) (-1,0)};

/* GOL rules. */
__constant uint2 live_rule = (uint2) (2, 3);
__constant uint2 dead_rule = (uint2) (3, 3);

/* How to read input image. */
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

/**
 * Kernel which performs GOL simulation.
 *
 * @param[in] CA input state.
 * @param[out] CA output state.
 * */
__kernel void ca(__read_only image2d_t in_img, __write_only image2d_t out_img) {

	int2 imdim = get_image_dim(in_img);
	int2 coord = (int2) (get_global_id(0), get_global_id(1));
	if (all(coord < imdim)) {
		uint4 neighs_state;
		uint neighs_alive = 0;
		uint4 state;
		uint alive;
		uint4 new_state = { 0xFF, 0, 0, 1};
		for(int i = 0; i < NUM_NEIGHS; ++i) {
			int2 n = coord + neighbors[i];
			n = select(n, n - imdim, n >= imdim);
			n = select(n, imdim - 1, n < 0);
			neighs_state = read_imageui(in_img, sampler, n);
			if (neighs_state.x == 0x0) neighs_alive++;
		}
		state = read_imageui(in_img, sampler, coord);
		alive = (state.x == 0x0);
		if ((alive && (neighs_alive >= live_rule.s0) && (neighs_alive <= live_rule.s1))
			|| (!alive && (neighs_alive >= dead_rule.s0) && (neighs_alive <= dead_rule.s1))) {
			new_state.x = 0x00;
		}
		write_imageui(out_img, coord, new_state);
	}
}
