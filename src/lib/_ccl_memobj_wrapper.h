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
 * Implementation of a wrapper class for OpenCL memory objects and declaration
 * of some of this methods. This file is only for building _cf4ocl_. Is is not
 * part of its public API.
 *
 * @internal
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_context_wrapper.h"
#include "_ccl_abstract_wrapper.h"

#ifndef __CCL_MEMOBJ_WRAPPER_H_
#define __CCL_MEMOBJ_WRAPPER_H_

/**
 * Base class for memory object wrappers, i.e., ::CCLBuffer and
 * ::CCLImage.
 *
 * @ingroup CCL_MEMOBJ_WRAPPER
 * @extends ccl_wrapper
 * */
struct ccl_memobj {

    /**
     * Parent wrapper object.
     * @private
     * */
    CCLWrapper base;

    /**
     * Context wrapper.
     * @private
     * */
    CCLContext * ctx;

};

/* Implementation of ccl_wrapper_release_fields() function for
 * ::CCLMemObj wrapper objects. */
void ccl_memobj_release_fields(CCLMemObj * mo);

#endif
