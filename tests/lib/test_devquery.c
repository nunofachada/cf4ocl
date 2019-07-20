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
 * Tests for device query module.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

/**
 * @internal
 *
 * @brief Tests devquery module helper functions.
 * */
static void helpers_test() {

    CCLPlatforms * platfs = NULL;
    CCLPlatform * p = NULL;
    CCLDevice * d = NULL;
    CCLErr * err = NULL;
    guint num_devs;
    guint num_platfs;
    CCLWrapperInfo * info;
    gchar param_value_str[CCL_TEST_DEVQUERY_MAXINFOLEN];

    /* Get platforms. */
    platfs = ccl_platforms_new(&err);
    if (err == NULL) {

        /* Number of platforms. */
        num_platfs = ccl_platforms_count(platfs);
        g_debug("* Found %d OpenCL platforms", num_platfs);

        /* Cycle through platforms. */
        for (guint i = 0; i < num_platfs; i++) {

            /* Get current platform. */
            p = ccl_platforms_get(platfs, i);
            g_debug(">> Platform %d:", i);

            /* Get number of devices. */
            num_devs = ccl_platform_get_num_devices(p, &err);

            /* Only test for device information if device count was
             * successfully obtained. */
            if (err != NULL) {
                g_test_message(
                    "Error obtaining number of devices for platform %d (%s).",
                    i, err->message);
                ccl_err_clear(&err);
            } else {

                g_debug("==== # Devs  : %d", num_devs);

                /* Cycle through devices in platform. */
                for (guint j = 0; j < num_devs; j++) {

                    /* Get current device. */
                    d = ccl_platform_get_device(p, j, &err);
                    g_assert_no_error(err);
                    g_debug("====== Device #%d", j);

                    for (gint k = 0; k < ccl_devquery_info_map_size; k++) {
                        info = ccl_device_get_info(
                            d, ccl_devquery_info_map[k].device_info, &err);
                        if (err == NULL) {
                            g_debug("\t%s : %s",
                                ccl_devquery_info_map[k].param_name,
                                ccl_devquery_info_map[k].format(
                                    info, param_value_str,
                                    CCL_TEST_DEVQUERY_MAXINFOLEN,
                                    ccl_devquery_info_map[k].units));
                        } else {
                            ccl_err_clear(&err);
                            g_debug("\t%s : %s",
                                ccl_devquery_info_map[k].param_name, "N/A");
                        }
                    }
                }
            }
        }

        /* Destroy list of platforms. */
        ccl_platforms_destroy(platfs);

    } else {

        /* Unable to get any OpenCL platforms, test can't pass. */
        g_test_message("Test failed due to following error: %s",
            err->message);
        g_test_fail();
    }

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Test the ccl_devquery_name function.
 * */
static void name_test() {

    /* Device information. */
    cl_device_info info;

    /* Test exact parameter name. */
    info = ccl_devquery_name("CL_DEVICE_ENDIAN_LITTLE");
    g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
    info = ccl_devquery_name("CL_DEVICE_EXTENSIONS");
    g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
    info = ccl_devquery_name("CL_DRIVER_VERSION");
    g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

    /* Test mixed parameter name. */
    info = ccl_devquery_name("cl_Device_Endian_Little");
    g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
    info = ccl_devquery_name("CL_device_Extensions");
    g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
    info = ccl_devquery_name("cl_DRIVer_version");
    g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

    /* Test lowercase parameter name without cl_device_ or cl_ prefix. */
    info = ccl_devquery_name("endian_little");
    g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
    info = ccl_devquery_name("extensions");
    g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
    info = ccl_devquery_name("driver_version");
    g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

    /* Test parameter name without CL_DEVICE_ or CL_ prefix. */
    info = ccl_devquery_name("ENDIAN_LITTLE");
    g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
    info = ccl_devquery_name("EXTENSIONS");
    g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
    info = ccl_devquery_name("DRIVER_VERSION");
    g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

    /* Test not found parameter name. */
    info = ccl_devquery_name("MOCK_PARAM_THAT_DOES_NOT_EXIST");
    g_assert_cmphex(info, ==, 0);

}

/**
 * @internal
 *
 * @brief Tests if the ccl_devquery_info_map array is well built,
 * namely (i) if the param_name fields are alphabetically ordered, and
 * (ii) if the size of the array corresponds to the
 * ccl_devquery_info_map_size variable.
 * */
static void infomap_test() {

    /* Determined size of info map. */
    gint imsize;

    /* Cycle through info map. */
    for (imsize = 0;
        ccl_devquery_info_map[imsize].param_name != NULL;
        imsize++) {

        if ((imsize > 0)
            &&
            (ccl_devquery_info_map[imsize].param_name != NULL)) {

            /* Test if parameter names are alphabetically ordered. */
            g_assert_cmpstr(
                ccl_devquery_info_map[imsize - 1].param_name,
                <,
                ccl_devquery_info_map[imsize].param_name);
        }
    }

    /* Test if size corresponds. */
    g_assert_cmpint(imsize, ==, ccl_devquery_info_map_size);
}

/**
 * @internal
 *
 * @brief Tests rarely used formatting functions or formatting functions used
 * in a rare way.
 * */
static void rare_test() {

    /* Variables. */
    CCLWrapperInfo info;
    ccl_devquery_format format_func;
    char out[CCL_TEST_DEVQUERY_MAXINFOLEN];

    /* 1. Test format hex function with units. */

    /* Get function, which is used for formatting CL_DEVICE_VENDOR_ID info. */
    format_func =
        ccl_devquery_info_map[ccl_devquery_get_index("VENDOR_ID")].format;

    /* Perform test. */
    cl_ushort hexval = 0xAB;
    info.size = sizeof(cl_ushort);
    info.value = (void *) &hexval;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "mockUnits");
    g_assert_cmpstr(out, ==, "0xab mockUnits");

    /* 2. Test format local memory type with option NONE, which will
     * probably never happen in a real situation. */

    /* Get function, which is used for formatting CL_DEVICE_LOCAL_MEM_TYPE
     * info. */
    format_func =
        ccl_devquery_info_map[ccl_devquery_get_index("LOCAL_MEM_TYPE")].format;

    /* Perform test. */
    cl_device_local_mem_type lmt = CL_NONE;
    info.size = sizeof(cl_device_local_mem_type);
    info.value = &lmt;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "");
    g_assert_cmpstr(out, ==, "NONE");

    /* 3. Test format of rarely implemented device partition properties. */

    /* Get function, used for formatting CL_DEVICE_PARTITION_PROPERTIES info. */
    format_func =
        ccl_devquery_info_map[
            ccl_devquery_get_index("PARTITION_PROPERTIES")].format;

    /* Perform test. */
    cl_device_partition_property part_props[] = {
        CL_DEVICE_PARTITION_EQUALLY,
        CL_DEVICE_PARTITION_BY_COUNTS,
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
        CL_DEVICE_PARTITION_EQUALLY_EXT,
        CL_DEVICE_PARTITION_BY_COUNTS_EXT,
        CL_DEVICE_PARTITION_BY_NAMES_EXT,
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT,
        -2, /* Unknown */
        0 };
    info.size = sizeof(part_props);
    info.value = part_props;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "");
    g_assert(g_strrstr(out, "EQUALLY") != NULL);
    g_assert(g_strrstr(out, "BY_COUNTS") != NULL);
    g_assert(g_strrstr(out, "BY_AFFINITY_DOMAIN") != NULL);
    g_assert(g_strrstr(out, "EQUALLY_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_COUNTS_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_NAMES_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_AFFINITY_DOMAIN_EXT") != NULL);
    g_assert(g_strrstr(out, "UNKNOWN") != NULL);

    /* 4. Test format of device partition properties in their optional
     * extension form. */

    /* Get function, used for formatting CL_DEVICE_PARTITION_TYPES_EXT info. */
    format_func =
        ccl_devquery_info_map[
            ccl_devquery_get_index("PARTITION_TYPES_EXT")].format;

    /* Perform test. */
    cl_device_partition_property_ext part_props_ext[] = {
        CL_DEVICE_PARTITION_EQUALLY_EXT,
        CL_DEVICE_PARTITION_BY_COUNTS_EXT,
        CL_DEVICE_PARTITION_BY_NAMES_EXT,
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT,
        -2, /* Unknown */
        0 };
    info.size = sizeof(part_props_ext);
    info.value = part_props_ext;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "");
    g_assert(g_strrstr(out, "EQUALLY_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_COUNTS_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_NAMES_EXT") != NULL);
    g_assert(g_strrstr(out, "BY_AFFINITY_DOMAIN_EXT") != NULL);
    g_assert(g_strrstr(out, "UNKNOWN") != NULL);

    /* 5. Test format of affinity domains when partitioning device by
     * affinity. */

    /* Get function, used for formatting CL_DEVICE_PARTITION_AFFINITY_DOMAIN
     * info. */
    format_func =
        ccl_devquery_info_map[
            ccl_devquery_get_index("PARTITION_AFFINITY_DOMAIN")].format;

    /* Perform test. */
    cl_device_affinity_domain ad =
        CL_DEVICE_AFFINITY_DOMAIN_NUMA | CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE;
    info.size = sizeof(cl_device_affinity_domain);
    info.value = &ad;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "");
    g_assert(g_strrstr(out, "NUMA") != NULL);
    g_assert(g_strrstr(out, "L1_CACHE") != NULL);
    g_assert_null(g_strrstr(out, "L2_CACHE"));
    g_assert_null(g_strrstr(out, "L3_CACHE"));
    g_assert_null(g_strrstr(out, "L4_CACHE"));

    /* 6. Test format of affinity domains when partitioning device by
     * affinity using the optional extension form. */

    /* Get function, used for formatting CL_DEVICE_AFFINITY_DOMAINS_EXT info. */
    format_func =
        ccl_devquery_info_map[
            ccl_devquery_get_index("AFFINITY_DOMAINS_EXT")].format;

    /* Perform test. */
    cl_device_partition_property_ext ppe[] = {
        CL_AFFINITY_DOMAIN_L1_CACHE_EXT,
        CL_AFFINITY_DOMAIN_L2_CACHE_EXT,
        CL_AFFINITY_DOMAIN_L3_CACHE_EXT,
        CL_AFFINITY_DOMAIN_L4_CACHE_EXT,
        CL_AFFINITY_DOMAIN_NUMA_EXT,
        -2, /* Unknown */
        CL_PROPERTIES_LIST_END_EXT };
    info.size = sizeof(ppe);
    info.value = ppe;
    format_func(&info, out, CCL_TEST_DEVQUERY_MAXINFOLEN, "");
    g_assert(g_strrstr(out, "NUMA_EXT") != NULL);
    g_assert(g_strrstr(out, "L1_CACHE_EXT") != NULL);
    g_assert(g_strrstr(out, "L2_CACHE_EXT") != NULL);
    g_assert(g_strrstr(out, "L3_CACHE_EXT") != NULL);
    g_assert(g_strrstr(out, "L4_CACHE_EXT") != NULL);
    g_assert(g_strrstr(out, "UNKNOWN") != NULL);
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

    g_test_add_func("/devquery/helpers", helpers_test);

    g_test_add_func("/devquery/name", name_test);

    g_test_add_func("/devquery/infomap", infomap_test);

    g_test_add_func("/devquery/rare", rare_test);

    return g_test_run();
}
