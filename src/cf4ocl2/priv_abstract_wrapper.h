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
 * Implementation of an abstract wrapper class. This file is only for
 * building _cf4ocl_. Is is not part of its public API.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_PRIV_ABSTRACT_WRAPPER_H_
#define _CCL_PRIV_ABSTRACT_WRAPPER_H_

/**
 * @internal
 * Information about wrapped OpenCL objects.
 * */
typedef struct ccl_wrapper_info_table CCLWrapperInfoTable;

/**
 * Base class for all OpenCL wrappers.
 * */
struct ccl_wrapper {

	/**
	 * The wrapped OpenCL object.
	 * @private
	 * */
	void* cl_object;

	/**
	 * Information about the wrapped OpenCL object.
	 * @private
	 * */
	CCLWrapperInfoTable* info;

	/**
	 * Reference count.
	 * @private
	 * */
	int ref_count;

};

#endif
