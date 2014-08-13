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
 * Implementation of a wrapper class and its methods for OpenCL sampler
 * objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "sampler_wrapper.h"

/** 
 * @addtogroup SAMPLER_WRAPPER
 * @{
 */

/** 
 * Sampler wrapper class.
 * 
 * @extends ccl_wrapper
 * */
struct ccl_sampler {

	/** 
	 * Parent wrapper object. 
	 * @private
	 * */
	CCLWrapper base;

};

/**
 * Get the sampler wrapper for the given OpenCL sampler.
 * 
 * If the wrapper doesn't exist, its created with a reference count 
 * of 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to directly wrap an OpenCL sampler in a 
 * ::CCLSampler wrapper object.
 * 
 * @protected @memberof ccl_sampler
 * 
 * @param[in] sampler The OpenCL sampler to be wrapped.
 * @return The ::CCLSampler wrapper for the given OpenCL sampler.
 * */
CCLSampler* ccl_sampler_new_wrap(cl_sampler sampler) {
	
	return (CCLSampler*) ccl_wrapper_new(
		(void*) sampler, sizeof(CCLSampler));
		
}

/** 
 * Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed.
 *
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr The sampler wrapper object.
 * */
void ccl_sampler_destroy(CCLSampler* smplr) {
	
	ccl_wrapper_unref((CCLWrapper*) smplr, sizeof(CCLSampler),
		NULL, (ccl_wrapper_release_cl_object) clReleaseSampler, NULL); 

}

/** @} */


