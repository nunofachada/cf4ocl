
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
 * Definition of a wrapper class and its methods for OpenCL sampler
 * objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_SAMPLER_WRAPPER_H_
#define _CCL_SAMPLER_WRAPPER_H_

#include "abstract_wrapper.h"

/**
 * @defgroup SAMPLER_WRAPPER Sampler wrapper
 *
 * A wrapper for handling samplers in OpenCL.
 * 
 * @todo Detailed description of module with code examples.
 * 
 * @{
 */

/** 
 * Sampler wrapper class
 * 
 * @extends ccl_wrapper
 * */
typedef struct ccl_sampler CCLSampler;

/* Get the sampler wrapper for the given OpenCL sampler. */
CCLSampler* ccl_sampler_new_wrap(cl_sampler sampler);

/* Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed. */
void ccl_sampler_destroy(CCLSampler* smplr);

/** @} */

#endif




