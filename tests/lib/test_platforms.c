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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @internal
 *
 * @file
 * Test the platforms class and its methods. Also tests the platform and
 * device wrapper classes.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include <glib/gstdio.h>
#include "test.h"

/* Max. length of information string. */
#define CCL_TEST_PLATFORMS_MAXINFOSTR 200

/* Test utility macro. Presents either the required information, or
 * the error message, if it occurred. Also frees the error object if
 * an error occurred. */
#define ccl_test_platforms_msg(base_msg, format, ...) \
    if (err == NULL) { \
        g_snprintf(info_str, CCL_TEST_PLATFORMS_MAXINFOSTR, \
            format, ##__VA_ARGS__); \
    } else { \
        g_snprintf(info_str, CCL_TEST_PLATFORMS_MAXINFOSTR, \
            "%s", err->message); \
        ccl_err_clear(&err); \
    } \
    g_debug("%s %s", base_msg, info_str);

static void ccl_test_platforms_check_error(CCLErr ** err) {
    gboolean status;
    if  (*err == NULL) {
        status = TRUE;
    } else if (((*err)->domain == CCL_ERROR)
            && ((*err)->code == CCL_ERROR_INFO_UNAVAILABLE_OCL)) {
        status = TRUE;
    } else if (((*err)->domain == CCL_OCL_ERROR)
            && ((*err)->code == CL_INVALID_VALUE)) {
        status = TRUE;
    } else {
        status = FALSE;
        g_debug("%s (Value=%d,Domain='%s')",
            (*err)->message, (*err)->code, g_quark_to_string((*err)->domain));
    }
    g_assert(status);
}

/**
 * @internal
 *
 * @brief Helper macro which returns a scalar information value casted
 * to specified scalar type.
 *
 * @param[in] info ::CCLWrapperInfo* information object.
 * @param[in] type Scalar type to which to cast value to.
 * @return The information value casted to the specified scalar type.
 * */
#define ccl_info_scalar(info, type) *((type *) (info)->value)

/**
 * @internal
 *
 * @brief Helper macro which returns an array information value casted
 * to specified array type.
 *
 * @param[in] info ::CCLWrapperInfo* information object.
 * @param[in] type Array (pointer) type to which to cast value to.
 * @return The information value casted to the specified array (pointer)
 * type.
 * */
#define ccl_info_array(info, type) ((type) (info)->value)

/**
 * @internal
 *
 * @brief Tests creation, getting info from and destruction of
 * platforms, platform and device wrapper objects.
 * */
static void create_info_destroy_test() {

    CCLPlatforms * platfs = NULL;
    CCLPlatform * p = NULL;
    CCLDevice * d = NULL;
    CCLDevice * const * ds = NULL;
    CCLErr * err = NULL;
    CCLWrapperInfo * info;
    gchar * platf_info, * aux_info;
    guint num_devs;
    guint num_platfs;
    gchar info_str[CCL_TEST_PLATFORMS_MAXINFOSTR];

    char * info_check_array;
    cl_uint info_check_scalar;
    cl_uint dev_ocl_ver;
    size_t info_size;

    /* Get platforms. */
    platfs = ccl_platforms_new(&err);
    g_assert_no_error(err);

    /* Number of platforms. */
    num_platfs = ccl_platforms_count(platfs);
    g_debug("* Found %d OpenCL platforms", num_platfs);

    /* Cycle through platforms. */
    for (guint i = 0; i < num_platfs; i++) {

        /* Get current platform. */
        p = ccl_platforms_get(platfs, i);
        g_debug(">> Platform %d:", i);

        /* Get platform profile. */
        platf_info = ccl_platform_get_info_string(
            p, CL_PLATFORM_PROFILE, &err);
        g_assert_no_error(err);
        ccl_test_platforms_msg("==== Profile :", "%s", platf_info);

        /* Get platform version. */
        platf_info = ccl_platform_get_info_string(
            p, CL_PLATFORM_VERSION, &err);
        g_assert_no_error(err);
        ccl_test_platforms_msg("==== Version :", "%s", platf_info);

        /* Get platform name. */
        platf_info = ccl_platform_get_info_string(
            p, CL_PLATFORM_NAME, &err);
        g_assert_no_error(err);
        ccl_test_platforms_msg("==== Name    :", "%s", platf_info);

        /* Get platform name directly with ccl_wrapper_info using the cache and
         * check that it's the pointer that was previously returned. */
        aux_info = (char *) ccl_wrapper_get_info_value(
            (CCLWrapper *) p, NULL, CL_PLATFORM_NAME, sizeof(char *),
            CCL_INFO_PLATFORM, CL_TRUE, &err);
        g_assert_no_error(err);
        g_assert_cmphex(
            GPOINTER_TO_SIZE(platf_info), ==, GPOINTER_TO_SIZE(aux_info));

        /* Get platform name information size and compare it with the real
         * thing. */
        info_size =  ccl_wrapper_get_info_size(
            (CCLWrapper *) p, NULL, CL_PLATFORM_NAME, 0,
            CCL_INFO_PLATFORM, CL_FALSE, &err);
        g_assert_no_error(err);
        g_assert_cmpuint(info_size / sizeof(char), ==, strlen(platf_info) + 1);

        /* Make an invalid info request. */
        platf_info = ccl_platform_get_info_string(
            p, 0x0 /* Invalid */, &err);
        g_assert(err != NULL);
        ccl_err_clear(&err);

        /* Get platform vendor. */
        platf_info = ccl_platform_get_info_string(
            p, CL_PLATFORM_VENDOR, &err);
        g_assert_no_error(err);
        ccl_test_platforms_msg("==== Vendor  :", "%s", platf_info);

        /* Get platform extensions. */
        platf_info = ccl_platform_get_info_string(
            p, CL_PLATFORM_EXTENSIONS, &err);
        g_assert_no_error(err);
        ccl_test_platforms_msg("==== Extens. :", "%s", platf_info);

        /* Get number of devices. */
        num_devs = ccl_platform_get_num_devices(p, &err);
        g_assert_no_error(err);

        g_debug("==== # Devs  : %d", num_devs);

        /* Cycle through devices in platform. */
        for (guint j = 0; j < num_devs; j++) {

            /* Get current device. */
            d = ccl_platform_get_device(p, j, &err);
            g_assert_no_error(err);
            g_debug("====== Device #%d", j);

            dev_ocl_ver = ccl_device_get_opencl_c_version(d, &err);
            g_assert_no_error(err);
            g_debug("...... OCL C Ver : %d", dev_ocl_ver);

            dev_ocl_ver = ccl_device_get_opencl_version(d, &err);
            g_assert_no_error(err);
            g_debug("...... OCL Ver : %d", dev_ocl_ver);

            info = ccl_device_get_info(d, CL_DEVICE_NAME, &err);
            g_assert_no_error(err);
            ccl_test_platforms_msg("...... Name :", "%s",
                ccl_info_array(info, char *));

            info = ccl_device_get_info(d, CL_DEVICE_ADDRESS_BITS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Address bits :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_AVAILABLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Device available :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(d, CL_DEVICE_BUILT_IN_KERNELS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Built-in kernels :", "%s",
                ccl_info_array(info, char *));

            info = ccl_device_get_info(d, CL_DEVICE_COMPILER_AVAILABLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Compiler available :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(d, CL_DEVICE_DOUBLE_FP_CONFIG, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... FP config (Double) :", "%lx",
                (unsigned long) ccl_info_scalar(info, cl_device_fp_config));

            info = ccl_device_get_info(d, CL_DEVICE_SINGLE_FP_CONFIG, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... FP config (Single) :", "%lx",
                (unsigned long) ccl_info_scalar(info, cl_device_fp_config));

            info = ccl_device_get_info(d, CL_DEVICE_HALF_FP_CONFIG, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... FP config (Half) :", "%lx",
                (unsigned long) ccl_info_scalar(info, cl_device_fp_config));

            info = ccl_device_get_info(d, CL_DEVICE_ENDIAN_LITTLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Endian little :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(
                d, CL_DEVICE_ERROR_CORRECTION_SUPPORT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Error correction support :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(
                d, CL_DEVICE_EXECUTION_CAPABILITIES, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Execution capabilities :", "%lx",
                (unsigned long) ccl_info_scalar(info, cl_device_exec_capabilities));

            info = ccl_device_get_info(d, CL_DEVICE_EXTENSIONS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Extensions :", "%s",
                ccl_info_array(info, char *));

            info = ccl_device_get_info(
                d, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Global mem. cache size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(
                d, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Global mem. cache type :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Global mem. cacheline size :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_GLOBAL_MEM_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Global mem. size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_HOST_UNIFIED_MEMORY, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Host unified memory :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE_SUPPORT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image support :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE2D_MAX_HEIGHT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image2d max. height :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE2D_MAX_WIDTH, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image2d max. width :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE3D_MAX_DEPTH, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image3d max. depth :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE3D_MAX_HEIGHT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image3d max. height :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_IMAGE3D_MAX_WIDTH, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image3d max. width :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(
                d, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image max. buffer size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(
                d, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Image max. array size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_LINKER_AVAILABLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Linker available :", "%s",
                ccl_info_scalar(info, cl_bool) ? "Yes" : "No");

            info = ccl_device_get_info(d, CL_DEVICE_LOCAL_MEM_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Local mem. size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_LOCAL_MEM_TYPE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Local mem. type :", "%d",
                ccl_info_scalar(info, cl_device_local_mem_type));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_CLOCK_FREQUENCY, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. clock frequency :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_COMPUTE_UNITS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. compute units :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_CONSTANT_ARGS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. constant args. :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. constant buffer size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_MEM_ALLOC_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. mem. alloc. size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_PARAMETER_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. parameter size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_READ_IMAGE_ARGS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. read image args. :", "%d",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_SAMPLERS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. samplers. :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_WORK_GROUP_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. work-group size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(
                d, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. work-item dims. :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MAX_WORK_ITEM_SIZES, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max wkitem sizes :", "%u, %u, %u",
                (unsigned int) ((size_t *) info->value)[0],
                (unsigned int) ((size_t *) info->value)[1],
                (unsigned int) ((size_t *) info->value)[2]);

            info = ccl_device_get_info(
                d, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Max. write image args. :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_MEM_BASE_ADDR_ALIGN, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Mem. base address align :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg(
                "...... Min. data type align size (deprecated in OpenCL 1.2) :",
                "%u", ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width char :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width short :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width int :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width long :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width float :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width double :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Native vector width half :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_OPENCL_C_VERSION, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... OpenCL C version :", "%s",
                ccl_info_array(info, char *));
            ccl_test_platforms_check_error(&err);

            info = ccl_device_get_info(d, CL_DEVICE_PLATFORM, &err);
            ccl_test_platforms_check_error(&err);
            g_assert_cmphex(
                GPOINTER_TO_SIZE(*((cl_platform_id *) info->value)),
                ==,
                GPOINTER_TO_SIZE(ccl_platform_unwrap(p)));

            info = ccl_device_get_info(d, CL_DEVICE_PARENT_DEVICE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Parent device :", "%p",
                *((void **) info->value));

            info = ccl_device_get_info(
                d, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Partition max. sub-devices :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PARTITION_PROPERTIES, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg(
                "...... Partition properties (only first) :", "%x",
                info != NULL ? (int) ((intptr_t *) info->value)[0] : 0);

            info = ccl_device_get_info(
                d, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Partition affinity domain :", "%lx",
                (unsigned long) ccl_info_scalar(info, cl_device_affinity_domain));

            info = ccl_device_get_info(d, CL_DEVICE_PARTITION_TYPE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Partition type (only first):", "%x",
                info != NULL ? (int) ((intptr_t *) info->value)[0] : 0);

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width char :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width short :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width int :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width long :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width float :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width double :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Preferred vector width half :", "%u",
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_PRINTF_BUFFER_SIZE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Printf buffer size :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(
                d, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg(
                "...... Preferred interop. user sync. :", "%s",
                ccl_info_scalar(info, cl_bool) ? "User sync." : "Device sync.");

            info = ccl_device_get_info(d, CL_DEVICE_PROFILE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Profile :", "%s",
                ccl_info_array(info, char *));

            info = ccl_device_get_info(
                d, CL_DEVICE_PROFILING_TIMER_RESOLUTION, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg(
                "...... Profiling timer resolution :", "%lu",
                (unsigned long) ccl_info_scalar(info, cl_ulong));

            info = ccl_device_get_info(d, CL_DEVICE_QUEUE_PROPERTIES, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Command queue properties :", "%s %s",
                (ccl_info_scalar(info, cl_command_queue_properties) &
                    CL_QUEUE_PROFILING_ENABLE)
                    ? "Profiling" : "",
                (ccl_info_scalar(info, cl_command_queue_properties) &
                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
                    ? "Out-Of-Order" : "");

            info = ccl_device_get_info(d, CL_DEVICE_TYPE, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Type :", "%s",
                ccl_devquery_type2str(ccl_info_scalar(info, cl_device_type)));

            info = ccl_device_get_info(d, CL_DEVICE_VENDOR, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Vendor :", "%s",
                ccl_info_array(info, char *));

            /* Special check for info_value_array macro. */
            info_check_array =
                ccl_device_get_info_array(d, CL_DEVICE_VENDOR, char, &err);
            ccl_test_platforms_check_error(&err);
            g_assert_cmpstr((char *) info->value, ==, info_check_array);

            info = ccl_device_get_info(d, CL_DEVICE_VENDOR_ID, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Vendor ID :", "%x",
                ccl_info_scalar(info, cl_uint));

            /* Special check for info_value_scalar macro. */
            info_check_scalar = ccl_device_get_info_scalar(
                d, CL_DEVICE_VENDOR_ID, cl_uint, &err);
            ccl_test_platforms_check_error(&err);
            g_assert_cmphex(info_check_scalar, ==,
                ccl_info_scalar(info, cl_uint));

            info = ccl_device_get_info(d, CL_DEVICE_VERSION, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Device version :", "%s",
                ccl_info_array(info, char *));

            info = ccl_device_get_info(d, CL_DRIVER_VERSION, &err);
            ccl_test_platforms_check_error(&err);
            ccl_test_platforms_msg("...... Driver version :", "%s",
                ccl_info_array(info, char *));
        }
    }

    /* Test get_all_devices method of platform module. */
    for (guint i = 0; i < ccl_platforms_count(platfs); i++) {
        p = ccl_platforms_get(platfs, i);
        ds = ccl_platform_get_all_devices(p, &err);
        g_assert_no_error(err);
        for (guint j = 0; j < ccl_platform_get_num_devices(p, &err); j++) {
            g_assert_no_error(err);
            d = ccl_platform_get_device(p, j, &err);
            g_assert_no_error(err);
            g_assert(d == ds[j]);
        }
        g_assert_no_error(err);
    }

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy list of platforms. */
    ccl_platforms_destroy(platfs);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Test increasing reference count of objects which compose
 * larger objects, then destroy the larger object and verify that
 * composing object still exists and must be freed by the function
 * which increase its reference count. This function tests the following
 * modules: platforms, platform and device wrappers.
 * */
static void ref_unref_test() {

    CCLPlatforms * platfs = NULL;
    CCLPlatform * p = NULL;
    CCLDevice * d = NULL;
    CCLErr * err = NULL;

    /* Get platforms. */
    platfs = ccl_platforms_new(&err);
    g_assert_no_error(err);

    /* Use first device of first platform. */
    p = ccl_platforms_get(platfs, 0);

    d = ccl_platform_get_device(p, 0, &err);
    g_assert_no_error(err);

    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) p), ==, 1);
    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) d), ==, 1);

    ccl_platform_ref(p);
    ccl_device_ref(d);

    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) p), ==, 2);
    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) d), ==, 2);

    ccl_platforms_destroy(platfs);

    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) p), ==, 1);
    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) d), ==, 2);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    ccl_platform_destroy(p);

    g_assert_cmpint(ccl_wrapper_ref_count((CCLWrapper *) d), ==, 1);

    ccl_device_destroy(d);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char ** argv) {

    g_test_init(&argc, &argv, NULL);

    g_test_add_func(
        "/wrappers/platforms/create-info-destroy",
        create_info_destroy_test);

    g_test_add_func(
        "/wrappers/platforms/ref-unref",
        ref_unref_test);

    return g_test_run();
}
