/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with cf4ocl. If not, see 
 * <http://www.gnu.org/licenses/>.
 * */
 
/** 
 * @file
 * 
 * Header for normalizing OpenCL versions within _cf4ocl_.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_OCLVERSIONS_H_
#define _CCL_OCLVERSIONS_H_

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
	#ifdef CL_VERSION_1_2
		#include <OpenCL/cl_dx9_media_sharing.h>
	#endif
#else
    #include <CL/opencl.h>
	#ifdef CL_VERSION_1_2
		#include <CL/cl_dx9_media_sharing.h>
	#endif
#endif


/* Define stuff for OpenCL implementations lower than 1.1 */
#ifndef CL_VERSION_1_1
	/* cl_kernel_work_group_info */
	#define CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE 0x11B3
	#define CL_KERNEL_PRIVATE_MEM_SIZE                  0x11B4
	/* cl_device_info */
	#define CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF       0x1034
	#define CL_DEVICE_HOST_UNIFIED_MEMORY               0x1035
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR          0x1036
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT         0x1037
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_INT           0x1038
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG          0x1039
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT         0x103A
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE        0x103B
	#define CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF          0x103C
	#define CL_DEVICE_OPENCL_C_VERSION                  0x103D	
	/* cl_device_fp_config - bitfield */
	#define CL_FP_SOFT_FLOAT                            (1 << 6)
	/* cl_command_type */
	#define CL_COMMAND_READ_BUFFER_RECT                 0x1201
	#define CL_COMMAND_WRITE_BUFFER_RECT                0x1202
	#define CL_COMMAND_COPY_BUFFER_RECT                 0x1203
	#define CL_COMMAND_USER                             0x1204
	#define CL_COMMAND_BARRIER                          0x1205
	
#endif

/* Define stuff for OpenCL implementations lower than 1.2 */
#ifndef CL_VERSION_1_2
	typedef intptr_t            cl_device_partition_property;
	typedef cl_bitfield         cl_device_affinity_domain;
	/* cl_device_type - bitfield */
	#define CL_DEVICE_TYPE_CUSTOM                       (1 << 4)
	/* cl_device_partition_property */
	#define CL_DEVICE_PARTITION_EQUALLY                 0x1086
	#define CL_DEVICE_PARTITION_BY_COUNTS               0x1087
	#define CL_DEVICE_PARTITION_BY_COUNTS_LIST_END      0x0
	#define CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN      0x1088
	/* cl_device_affinity_domain */
	#define CL_DEVICE_AFFINITY_DOMAIN_NUMA                     (1 << 0)
	#define CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE                 (1 << 1)
	#define CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE                 (1 << 2)
	#define CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE                 (1 << 3)
	#define CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE                 (1 << 4)
	#define CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE       (1 << 5)
	/* cl_device_info */
	#define CL_DEVICE_LINKER_AVAILABLE                  0x103E
	#define CL_DEVICE_BUILT_IN_KERNELS                  0x103F
	#define CL_DEVICE_IMAGE_MAX_BUFFER_SIZE             0x1040
	#define CL_DEVICE_IMAGE_MAX_ARRAY_SIZE              0x1041
	#define CL_DEVICE_PARENT_DEVICE                     0x1042
	#define CL_DEVICE_PARTITION_MAX_SUB_DEVICES         0x1043
	#define CL_DEVICE_PARTITION_PROPERTIES              0x1044
	#define CL_DEVICE_PARTITION_AFFINITY_DOMAIN         0x1045
	#define CL_DEVICE_PARTITION_TYPE                    0x1046
	#define CL_DEVICE_REFERENCE_COUNT                   0x1047
	#define CL_DEVICE_PREFERRED_INTEROP_USER_SYNC       0x1048
	#define CL_DEVICE_PRINTF_BUFFER_SIZE                0x1049
	#define CL_DEVICE_IMAGE_PITCH_ALIGNMENT             0x104A
	#define CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT      0x104B
	/* cl_command_type */
	#define CL_COMMAND_MIGRATE_MEM_OBJECTS              0x1206
	#define CL_COMMAND_FILL_BUFFER                      0x1207
	#define CL_COMMAND_FILL_IMAGE                       0x1208
	#define CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR        0x202B
	#define CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR        0x202C	
	
#endif	

/* Define stuff for OpenCL implementations lower than 2.0 */
#ifndef CL_VERSION_2_0

	typedef cl_bitfield         cl_device_svm_capabilities;
	/* cl_command_type */
	#define CL_COMMAND_SVM_FREE                         0x1209
	#define CL_COMMAND_SVM_MEMCPY                       0x120A
	#define CL_COMMAND_SVM_MEMFILL                      0x120B
	#define CL_COMMAND_SVM_MAP                          0x120C
	#define CL_COMMAND_SVM_UNMAP                        0x120D
	/* cl_device_info */
	#define CL_DEVICE_QUEUE_ON_HOST_PROPERTIES              0x102A
	#define CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS             0x104C
	#define CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE              0x104D
	#define CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES            0x104E
	#define CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE        0x104F
	#define CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE              0x1050
	#define CL_DEVICE_MAX_ON_DEVICE_QUEUES                  0x1051
	#define CL_DEVICE_MAX_ON_DEVICE_EVENTS                  0x1052
	#define CL_DEVICE_SVM_CAPABILITIES                      0x1053
	#define CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE  0x1054
	#define CL_DEVICE_MAX_PIPE_ARGS                         0x1055
	#define CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS          0x1056
	#define CL_DEVICE_PIPE_MAX_PACKET_SIZE                  0x1057
	#define CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT   0x1058
	#define CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT     0x1059
	#define CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT      0x105A
	/* cl_device_svm_capabilities */
	#define CL_DEVICE_SVM_COARSE_GRAIN_BUFFER           (1 << 0)
	#define CL_DEVICE_SVM_FINE_GRAIN_BUFFER             (1 << 1)
	#define CL_DEVICE_SVM_FINE_GRAIN_SYSTEM             (1 << 2)
	#define CL_DEVICE_SVM_ATOMICS                       (1 << 3)

#endif	

/* Some of these query constants may not be defined in standard 
 * OpenCL headers, so we defined them here if necessary. */
#ifndef CL_DEVICE_TERMINATE_CAPABILITY_KHR
	#define CL_DEVICE_TERMINATE_CAPABILITY_KHR          0x200F
#endif
#ifndef CL_COMMAND_EGL_FENCE_SYNC_OBJECT_KHR
	#define CL_COMMAND_EGL_FENCE_SYNC_OBJECT_KHR  0x202F
#endif
#ifndef CL_COMMAND_ACQUIRE_EGL_OBJECTS_KHR
	#define CL_COMMAND_ACQUIRE_EGL_OBJECTS_KHR    0x202D
#endif
#ifndef CL_COMMAND_RELEASE_EGL_OBJECTS_KHR
	#define CL_COMMAND_RELEASE_EGL_OBJECTS_KHR    0x202E
#endif
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
	#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
#endif
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
	#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001
#endif
#ifndef CL_DEVICE_REGISTERS_PER_BLOCK_NV
	#define CL_DEVICE_REGISTERS_PER_BLOCK_NV            0x4002
#endif
#ifndef CL_DEVICE_WARP_SIZE_NV
	#define CL_DEVICE_WARP_SIZE_NV                      0x4003
#endif
#ifndef CL_DEVICE_GPU_OVERLAP_NV
	#define CL_DEVICE_GPU_OVERLAP_NV                    0x4004
#endif
#ifndef CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV
	#define CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV            0x4005
#endif
#ifndef CL_DEVICE_INTEGRATED_MEMORY_NV
	#define CL_DEVICE_INTEGRATED_MEMORY_NV              0x4006
#endif
#ifndef CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR
	#define CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR         0x4017
#endif
#ifndef CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR
	#define CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR         0x4018
#endif
#ifndef CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR
	#define CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR         0x4020
#endif
#ifndef CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR
	#define CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR         0x4021
#endif 
#ifndef CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT
	#define CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT           0x4032
#endif
#ifndef CL_DEVICE_TOPOLOGY_AMD
	#define CL_DEVICE_TOPOLOGY_AMD                      0x4037
#endif
#ifndef CL_DEVICE_BOARD_NAME_AMD
	#define CL_DEVICE_BOARD_NAME_AMD                    0x4038
#endif
#ifndef CL_DEVICE_GLOBAL_FREE_MEMORY_AMD
	#define CL_DEVICE_GLOBAL_FREE_MEMORY_AMD            0x4039
#endif
#ifndef CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD
	#define CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD         0x4040
#endif
#ifndef CL_DEVICE_SIMD_WIDTH_AMD
	#define CL_DEVICE_SIMD_WIDTH_AMD                    0x4041
#endif
#ifndef CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD
	#define CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD        0x4042
#endif
#ifndef CL_DEVICE_WAVEFRONT_WIDTH_AMD
	#define CL_DEVICE_WAVEFRONT_WIDTH_AMD               0x4043
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD           0x4044
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD      0x4045
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD 0x4046
#endif
#ifndef CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD
	#define CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD   0x4047
#endif
#ifndef CL_DEVICE_LOCAL_MEM_BANKS_AMD
	#define CL_DEVICE_LOCAL_MEM_BANKS_AMD               0x4048
#endif
#ifndef CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD
	#define CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD        0x4049
#endif
#ifndef CL_DEVICE_SPIR_VERSIONS
	#define CL_DEVICE_SPIR_VERSIONS                     0x40E0
#endif
#ifndef CL_DEVICE_PARENT_DEVICE_EXT
	#define CL_DEVICE_PARENT_DEVICE_EXT                 0x4054
#endif
#ifndef CL_DEVICE_PARTITION_TYPES_EXT
	#define CL_DEVICE_PARTITION_TYPES_EXT               0x4055
#endif
#ifndef CL_DEVICE_AFFINITY_DOMAINS_EXT
	#define CL_DEVICE_AFFINITY_DOMAINS_EXT              0x4056
#endif
#ifndef CL_DEVICE_REFERENCE_COUNT_EXT
	#define CL_DEVICE_REFERENCE_COUNT_EXT               0x4057
#endif
#ifndef CL_DEVICE_PARTITION_STYLE_EXT
	#define CL_DEVICE_PARTITION_STYLE_EXT               0x4058
#endif
#ifndef CL_DEVICE_EXT_MEM_PADDING_IN_BYTES_QCOM
	#define CL_DEVICE_EXT_MEM_PADDING_IN_BYTES_QCOM   0x40A0      
#endif
#ifndef CL_DEVICE_PAGE_SIZE_QCOM
	#define CL_DEVICE_PAGE_SIZE_QCOM                  0x40A1
#endif

#endif /* _CCL_OCLVERSIONS_H_ */
