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
 * Implementation of an abstract device container wrapper class. This
 * file is only for building _cf4ocl_. Is is not part of its public API.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_PRIV_ABSTRACT_DEV_CONTAINER_WRAPPER_H_
#define _CCL_PRIV_ABSTRACT_DEV_CONTAINER_WRAPPER_H_

#include "priv_abstract_wrapper.h"

/**
 * Base class for wrappers which contain devices, i.e.,
 * ::CCLPlatform, ::CCLProgram and ::CCLContext. This class extends
 * ::CCLWrapper.
 *
 * @extends ccl_wrapper
 * */
struct ccl_dev_container {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

	/**
	 * Number of devices in context (can be lazy initialized).
	 * @private
	 * */
	cl_uint num_devices;

	/**
	 * Devices in context (can be lazy initialized).
	 * @private
	 * */
	CCLDevice** devices;

};

#endif
