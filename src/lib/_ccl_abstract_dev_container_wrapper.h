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
 * @internal
 * @file
 *
 * Implementation of an abstract device container wrapper class and
 * specification of its methods. This file is only for building _cf4ocl_. Is is
 * not part of its public API.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef __CCL_ABSTRACT_DEV_CONTAINER_WRAPPER_H_
#define __CCL_ABSTRACT_DEV_CONTAINER_WRAPPER_H_

#include "ccl_oclversions.h"
#include "ccl_common.h"
#include "ccl_errors.h"
#include "ccl_device_wrapper.h"
#include "ccl_abstract_wrapper.h"
#include "_ccl_abstract_wrapper.h"

/**
 * Base class for wrappers which contain devices, i.e., ::CCLPlatform,
 * ::CCLProgram and ::CCLContext. This class extends ::CCLWrapper.
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
     * Number of devices in container (can be lazy initialized).
     * @private
     * */
    cl_uint num_devices;

    /**
     * Devices in container (can be lazy initialized).
     * @private
     * */
    CCLDevice** devices;

};

/**
 * @internal
 * Returns the list of cl_device_id OpenCL objects in the
 * wrapped OpenCL device container object (i.e. `cl_platform_id`,
 * `cl_program` and `cl_context`). This is an abstract function prototype
 * which must be implemented by device container wrappers, i.e.,
 * ::CCLPlatform, ::CCLProgram and ::CCLContext.
 *
 * @protected @memberof ccl_dev_container
 *
 * @param[in] devcon The device container wrapper.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return List of `cl_device_id` OpenCL objects in the OpenCL device container
 * object wrapped by the given device container wrapper.
 * */
typedef CCLWrapperInfo * (*ccl_dev_container_get_cldevices)(
    CCLDevContainer * devcon, CCLErr ** err);

/* Release the devices held by the given #CCLDevContainer
 * object. */
void ccl_dev_container_release_devices(CCLDevContainer * devcon);

/* Get all ::CCLDevice wrappers in device container. */
CCLDevice * const * ccl_dev_container_get_all_devices(
    CCLDevContainer * devcon,
    ccl_dev_container_get_cldevices get_devices, CCLErr ** err);

/* Get ::CCLDevice wrapper at given index. */
CCLDevice* ccl_dev_container_get_device(CCLDevContainer * devcon,
    ccl_dev_container_get_cldevices get_devices,
    cl_uint index, CCLErr** err);

/* Return number of devices in device container. */
cl_uint ccl_dev_container_get_num_devices(CCLDevContainer * devcon,
    ccl_dev_container_get_cldevices get_devices, CCLErr ** err);

#endif


