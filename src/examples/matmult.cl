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
 * Matrix multiplication example OpenCL kernels based on the
 * [CUDA best practices guide](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html).
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

/**
 * Matmult kernel non-optimized.
 * 
 * Based on [Unoptimized matrix multiplication](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__unoptimized-matrix-multiplication).
 * 
 * @param[in] A Matrix A.
 * @param[in] B Matrix B.
 * @param[out] C Result matrix.
 * @param[in] dimsA Dimensions of matrix A.
 * @param[in] dimsB Dimensions of matrix B.
 */
__kernel void matmult0(__global int * A, __global int * B, 
	__global int * C, __private int2 dimsA, __private int2 dimsB) {
		
	/* Matrix position for this work-item */
	uint col = get_global_id(0);
	uint row = get_global_id(1); 
	
	/* Multiply! */
	if ((row < dimsA.y) && (col < dimsB.x)) {
		int sum = 0;
		for (uint i = 0; i < dimsA.x; i++) {
			sum += A[row * dimsA.x + i] * B[i * dimsB.x + col];
		}
		C[row * dimsB.x + col] = sum;
	}
}

/**
 * Matmult kernel optimized for local memory (matrix A reads).
 * 
 * Based on [Using shared memory to improve the global memory load efficiency in matrix multiplication](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__using-shared-memory-improve-global-memory-load-efficiency-matrix-multiplication).
 * 
 * @param[in] A Matrix A.
 * @param[in] B Matrix B.
 * @param[out] C Result matrix.
 * @param[in] dimsA Dimensions of matrix A.
 * @param[in] dimsB Dimensions of matrix B.
 * @param[in] tileOfA Local memory used to improve matrix multiplication.
 * */
__kernel void matmult1(__global int * A, __global int * B, __global int * C, __private int2 dimsA, __private int2 dimsB, __local int * tileOfA)
{
	/* Global matrix position for this work-item */
	uint gCol = get_global_id(0);
	uint gRow = get_global_id(1); 

	/* Local matrix position for this work-item */
	uint lCol = get_local_id(0);
	uint lRow = get_local_id(1); 
		
	/* Load of a tile of A into local memory (coalesced for width of work-group, localCols) */
	uint localCols = min((uint) get_local_size(0), (uint) dimsB.x);
	uint loops = (dimsA.x % localCols == 0) ? (dimsA.x / localCols) : (dimsA.x / localCols + 1);
	for (uint i = 0; i < loops; i++) {
		uint tileCol = i * localCols + lCol;
		if (tileCol < dimsA.x)
			tileOfA[lRow * dimsA.x + tileCol] = A[gRow * dimsA.x + tileCol];
	}
		
	/*/ Sync. locally */
	barrier(CLK_LOCAL_MEM_FENCE);
		
	/* Check if this work-item is within bounds of C to perform multiplication */
	if ((gRow < dimsA.y) && (gCol < dimsB.x)) {

		/* Multiply! */
		int sum = 0;
		for (uint i = 0; i < dimsA.x; i++) {
			sum += tileOfA[lRow * dimsA.x + i] * B[i * dimsB.x + gCol];
		}
		C[gRow * dimsB.x + gCol] = sum;
	}
}

/**
 * Matmult kernel optimized for local memory (matrix A reads, matrix B reads).
 * 
 * Based on [Improvement by reading additional data into shared memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__improvement-reading-additional-data-shared-memory).
 * 
 * @param[in] A Matrix A.
 * @param[in] B Matrix B.
 * @param[out] C Result matrix.
 * @param[in] dimsA Dimensions of matrix A.
 * @param[in] dimsB Dimensions of matrix B.
 * @param[in] tileOfA Local memory used to improve matrix multiplication.
 * @param[in] tileOfB Additional local memory used to improve matrix 
 * multiplication.
 */
__kernel void matmult2(__global int * A, __global int * B, __global int * C, __private int2 dimsA, __private int2 dimsB, __local int * tileOfA, __local int * tileOfB)
{
	/* Variable used to control reads from global memory into local memory */
	uint loops;
	
	/* Global matrix position for this work-item */
	uint gCol = get_global_id(0);
	uint gRow = get_global_id(1); 

	/* Local matrix position for this work-item */
	uint lCol = get_local_id(0);
	uint lRow = get_local_id(1); 
		
	/* Load of a tile of A into local memory (coalesced for width of work-group, localCols) */
	uint localCols = min((uint) get_local_size(0), (uint) dimsB.x);
	loops = (dimsA.x % localCols == 0) ? (dimsA.x / localCols) : (dimsA.x / localCols + 1);
	for (uint i = 0; i < loops; i++) {
		uint tileCol = i * localCols + lCol;
		if (tileCol < dimsA.x)
			tileOfA[lRow * dimsA.x + tileCol] = A[gRow * dimsA.x + tileCol];
	}
		
	/* Load of a tile of B into local memory (coalesced for width of work-group, localCols) */
	uint localRows = min((uint) get_local_size(1), (uint) dimsB.y);
	loops = (dimsB.y % localRows == 0) ? (dimsB.y / localRows) : (dimsB.y / localRows + 1);
	for (uint i = 0; i < loops; i++) {
		uint localPos = i * localCols * localRows + lRow * localCols + lCol;
		uint globalRow = i * localRows + lRow;
		uint globalCol = get_group_id(0) * localCols + lCol;
		if ((globalRow < dimsB.y) && (globalCol < dimsB.x))
			tileOfB[localPos] = B[globalRow * dimsB.x + globalCol];
	}

	/* Sync. locally */
	barrier(CLK_LOCAL_MEM_FENCE);
		
	/* Check if this work-item is within bounds of C to perform multiplication */
	if ((gRow < dimsA.y) && (gCol < dimsB.x)) {

		/* Multiply! */
		int sum = 0;
		for (uint i = 0; i < dimsA.x; i++) {
			sum += tileOfA[lRow * dimsA.x + i] * tileOfB[i * localCols + lCol];
		}
		C[gRow * dimsB.x + gCol] = sum;
	}
}

/**
 * Non-optimized kernel for matrix transpose multiplication.
 * 
 * Based on [Unoptimized handling of strided accesses to global memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-aa__unoptimized-handling-strided-accesses-global-memory).
 * 
 * @param[in] A Matrix A.
 * @param[out] C Result matrix.
 * @param[in] dimsA Dimensions of matrix A.
 */
__kernel void matmult3(__global int * A, __global int * C, __private int2 dimsA)
{
	/* Matrix position for this work-item */
	uint row = get_global_id(1); 
	uint col = get_global_id(0);
	
	/* Multiply! */
	if ((row < dimsA.y) && (col < dimsA.y)) {
		int sum = 0;
		for (uint i = 0; i < dimsA.x; i++) {
			sum += A[row * dimsA.x + i] * A[col * dimsA.x + i];
		}
		C[row * dimsA.y + col] = sum;
	}
}

/**
 * Optimized kernel for matrix transpose multiplication.
 * 
 * Based on [An optimized handling of strided accesses using coalesced reads from global memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-aa__optimized-version-coalesced-reads-global-memory).
 * 
 * @param[in] A Matrix A.
 * @param[out] C Result matrix.
 * @param[in] dimsA Dimensions of matrix A.
 * @param[in] tileOfA Local memory used to improve matrix multiplication.
 * @param[in] tileOfAT Additional local memory used to improve matrix 
 * multiplication.
 */
__kernel void matmult4(__global int * A, __global int * C, __private int2 dimsA, __local int * tileOfA, __local int * tileOfAT)
{
	/* Variable used to control reads from global memory into local memory */
	uint loops;

	/* Global matrix position for this work-item */
	uint gRow = get_global_id(1); 
	uint gCol = get_global_id(0);

	/* Local matrix position for this work-item */
	uint lRow = get_local_id(1); 
	uint lCol = get_local_id(0);
	
	/* Load of a tile of A into local memory (coalesced for width of work-group, localCols) */
	uint localCols = min((uint) get_local_size(0), (uint) dimsA.y);
	loops = (dimsA.x % localCols == 0) ? (dimsA.x / localCols) : (dimsA.x / localCols + 1);
	for (uint i = 0; i < loops; i++) {
		uint tileCol = i * localCols + lCol;
		if (tileCol < dimsA.x)
			tileOfA[lRow * dimsA.x + tileCol] = A[gRow * dimsA.x + tileCol];
	}

	/* Load of a tile of A^T into local memory (coalesced for width of work-group, localCols) */
	uint localRows = min((uint) get_local_size(1), (uint) dimsA.y);
	loops = (dimsA.x % localRows == 0) ? (dimsA.x / localRows) : (dimsA.x / localRows + 1);
	for (uint i = 0; i < loops; i++) {
		uint stripSize = dimsA.x * localCols;
		uint globalPos = stripSize * get_group_id(0);
		uint localPos = i * localCols * localRows;
		uint localIndex = lRow * localCols + lCol;
		uint globalIndex = globalPos + localPos + localIndex;
		if (globalIndex < stripSize * (get_group_id(0) + 1))
			tileOfAT[localPos + localIndex] = A[globalIndex];
	}

	/* Sync. locally */
	barrier(CLK_LOCAL_MEM_FENCE);
		
	/* Check if this work-item is within bounds of C to perform multiplication */
	if ((gRow < dimsA.y) && (gCol < dimsA.y)) {

		/* Multiply! */
		int sum = 0;
		for (uint i = 0; i < dimsA.x; i++) {
			sum += tileOfA[lRow * dimsA.x + i] * tileOfAT[lCol * dimsA.x + i];
		}
		C[gRow * dimsA.y + gCol] = sum; 
	}
}
