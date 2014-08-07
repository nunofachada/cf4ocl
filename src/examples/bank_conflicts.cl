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
 * Bank conflicts test example OpenCL kernels.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

/**
 * Kernel for testing bank conflicts.
 * 
 * @param[in,out] globalData Pointer to global data.
 * @param[in] localData Pointer to allocated local memory.
 * @param[in] stride Control level of bank conflicts.
 * */
__kernel void bankconf(__global int * globalData, __local int * localData, __private uint stride)
{
	/* Data position for this work-item */
	uint gRow = get_global_id(1); 
	uint gCol = get_global_id(0);
	uint lRow = get_local_id(1); 
	uint lCol = get_local_id(0);
	uint gTotCols = get_global_size(0);
	uint lTotCols = get_local_size(0);
	uint lTotElems = get_local_size(0) * get_local_size(1);
	uint gIndex = gRow * gTotCols + gCol;
	uint lIndex = lRow * lTotCols + lCol;
		
	/* Copy to local memory */
	localData[lIndex] = globalData[gIndex];
	
	/* Sync. locally */
	barrier(CLK_LOCAL_MEM_FENCE);
	
	/* Do some conflicts! */
	int sum = 0;
	/* Each thread will sum a number of elements in the local data. */
	for (uint i = 0; i < lTotCols; i++) {
		/* Get index of next element to sum. */
		uint idx = lIndex * stride + i;
		/* If index bigger than local memory size, wrap around. */
		if (idx >= lTotElems)
			idx = idx - lTotElems;
		/* Perform sum. */
		sum += localData[idx];
	}

	/* Copy to global memory */
	globalData[gIndex] = sum;
}
