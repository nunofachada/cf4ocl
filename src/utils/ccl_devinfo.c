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
 * @file
 * Utility to query OpenCL platforms and devices.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/**
 * @page ccl_devinfo
 *
 * @brief Utility to query OpenCL platforms and devices.
 *
 * SYNOPSIS
 * ========
 *
 * **ccl_devinfo** [_OPTIONS_]...
 *
 * DESCRIPTION
 * ===========
 *
 * The `ccl_devinfo` program prints information about the OpenCL platforms
 * and devices available on the system. By default, only basic device
 * information, such as type, vendor or OpenCL version, is shown, allowing
 * the user to quickly assess what platforms and devices are available.
 *
 * All the available device information items can be shown with the
 * <strong>-a</strong> or <strong>--all</strong> options. The
 * <strong>-c</strong> <em>cl_device_info</em> option, or its long variant,
 * <strong>--custom=</strong><em>cl_device_info</em>, allows the user to specify
 * a partial or complete `cl_device_info` parameter name. All matching
 * parameters will be printed. This option can be used several times.
 *
 * The <strong>-l</strong> flag, or its long variant <strong>--list</strong>,
 * tells <strong>ccl_devinfo</strong> to list all known device information
 * parameters. The <strong>-n</strong> or <strong>--notfound</strong> flags will
 * force the program to show known parameters even if the respective information
 * is not provided by the device.
 *
 * Additional information about each listed parameter can be shown with the
 * <strong>-v</strong> or <strong>--verbose</strong> flags.
 *
 * The **ccl_devinfo** utility can also narrow down information to specific
 * platforms and devices with the <strong>-p</strong> and <strong>-d</strong>
 * options, respectively (or their long variants, <strong>--platform</strong>
 * and <strong>--device</strong>). These options accept the platform index or
 * the (platform-wise) device index to perform the selection. The
 * <strong>-o</strong> or <strong>--no-platf</strong> option ignores platforms
 * and lists available devices independently of their platform. In this case,
 * the <strong>-d</strong> option will indicate the system-wise device index.
 *
 * <dl>
 * <dt>-a, --all</dt>
 * <dd>Show all the available device information</dd>
 * <dt>-b, --basic</dt>
 * <dd>Show basic device information (default)</dd>
 * <dt>-c, --custom=cl_device_info</dt>
 * <dd>Show specific information, repeat as necessary</dd>
 * <dt>-d, --device=dev_idx</dt>
 * <dd>Specify the index of a device to query</dd>
 * <dt>-o, --no-platf</dt>
 * <dd>Ignore platforms, device index reports to all devices available in the
 * system</dd>
 * <dt>-l, --list</dt>
 * <dd>List known information parameters</dd>
 * <dt>-p, --platform=platf_idx</dt>
 * <dd>Specify the index of a platform to query</dd>
 * <dt>-n, --notfound</dt>
 * <dd>Show known parameters even if not found in device</dd>
 * <dt>-v, --verbose</dt>
 * <dd>Show description of each parameter</dd>
 * <dt>--version</dt>
 * <dd>Output version information and exit</dd>
 * <dt>-h, --help, -?</dt>
 * <dd>Show help options and exit</dd>
 * </dl>
 *
 * AUTHOR
 * ======
 *
 * Written by Nuno Fachada.
 *
 * REPORTING BUGS
 * ==============
 *
 * * Report ccl_devinfo bugs at https://github.com/fakenmc/cf4ocl/issues
 * * cf4ocl home page: http://fakenmc.github.io/cf4ocl/
 * * cf4ocl wiki: https://github.com/fakenmc/cf4ocl/wiki
 *
 * COPYRIGHT
 * =========
 *
 * Copyright (C) 2019 Nuno Fachada<br/>
 * License GPLv3+: GNU GPL version 3 or later
 * <http://gnu.org/licenses/gpl.html>.<br/>
 * This is free software: you are free to change and redistribute it.<br/>
 * There is NO WARRANTY, to the extent permitted by law.
 *
 * */

#include "ccl_utils.h"

#ifndef CCL_DEVINFO_OUT
    /** Default device information output stream. */
    #define CCL_DEVINFO_OUT stdout
#endif

#ifndef CCL_DEVINFO_NA
    #define CCL_DEVINFO_NA "N/A"
#endif

/** Program description. */
#define CCL_DEVINFO_DESCRIPTION "Utility for querying OpenCL " \
    "platforms and devices"

/** Maximum length of device information output, per parameter. */
#define CCL_DEVINFO_MAXINFOLEN 500

/* Command line arguments and respective default values. */
static gboolean opt_all = FALSE;
static gboolean opt_basic = TRUE; /* Default. */
static gboolean no_platf = FALSE;
static gchar ** opt_custom = NULL;
static guint opt_platf = G_MAXUINT;
static guint opt_dev = G_MAXUINT;
static gboolean opt_nfound = FALSE;
static gboolean opt_verb = FALSE;
static gboolean opt_list = FALSE;
static gboolean version = FALSE;

/* Valid command line options. */
static GOptionEntry entries[] = {
    {"all",      'a', 0, G_OPTION_ARG_NONE,               &opt_all,
     "Show all the available device information",         NULL},
    {"basic",    'b', 0, G_OPTION_ARG_NONE,               &opt_basic,
     "Show basic device information (default)",           NULL},
    {"custom",   'c', 0, G_OPTION_ARG_STRING_ARRAY,       &opt_custom,
     "Show specific information, repeat as necessary",    "cl_device_info"},
    {"device",   'd', 0, G_OPTION_ARG_INT,                &opt_dev,
     "Specify the index of a device to query",            "dev_idx"},
    {"no-platf", 'o', 0, G_OPTION_ARG_NONE,               &no_platf,
     "Ignore platforms, device index reports to all "
     "devices available in the system",                   NULL},
    {"list",     'l', 0, G_OPTION_ARG_NONE,               &opt_list,
     "List known information parameters",                 NULL},
    {"platform", 'p', 0, G_OPTION_ARG_INT,                &opt_platf,
     "Specify the index of a platform to query",          "platf_idx"},
    {"notfound", 'n', 0, G_OPTION_ARG_NONE,               &opt_nfound,
     "Show known parameters even if not found in device", NULL},
    {"verbose",  'v', 0, G_OPTION_ARG_NONE,               &opt_verb,
     "Show description of each parameter",                NULL},
    {"version",    0, 0, G_OPTION_ARG_NONE,               &version,
     "Output version information and exit",               NULL},
    { NULL, 0, 0, 0, NULL, NULL, NULL }
};

/* Information queried for basic CLI option. */
static gchar * basic_info[] = {
    "TYPE",
    "VENDOR",
    "OPENCL_C_VERSION",
    "MAX_COMPUTE_UNITS",
    "GLOBAL_MEM_SIZE",
    "MAX_MEM_ALLOC_SIZE",
    "LOCAL_MEM_SIZE",
    "LOCAL_MEM_TYPE",
    "MAX_WORK_GROUP_SIZE",
    NULL
};

/* Macro to invoke correct function for the specified device information. */
#define ccl_devinfo_show_device_info(d) \
    if (opt_all) ccl_devinfo_show_device_info_all(d); \
    else if (opt_custom) ccl_devinfo_show_device_info_custom(d); \
    else ccl_devinfo_show_device_info_basic(d);

/**
 * Parse and verify command line arguments.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @param[out] err Return location for a CCLErr, or `NULL` if error
 * reporting is to be ignored.
 * */
void ccl_devinfo_args_parse(int argc, char * argv[], CCLErr ** err) {

    /* Make sure err is NULL or it is not set. */
    g_return_if_fail(err == NULL || *err == NULL);

    /* Command line options context. */
    GOptionContext * context = NULL;

    /* Create parsing context. */
    context = g_option_context_new(" - " CCL_DEVINFO_DESCRIPTION);

    /* Add acceptable command line options to context. */
    g_option_context_add_main_entries(context, entries, NULL);

    /* Use context to parse command line options. */
    g_option_context_parse(context, &argc, &argv, err);
    ccl_if_err_goto(*err, error_handler);

    /* If we get here, no need for error treatment, jump to cleanup. */
    g_assert(*err == NULL);
    goto cleanup;

error_handler:

    /* If we got here, everything is OK. */
    g_assert(*err != NULL);

cleanup:

    /* Free context. */
    if (context) g_option_context_free(context);

    /* Return. */
    return;
}

/**
 * Show platform information.
 *
 * @param[in] p Platform wrapper object.
 * @param[in] idx Platform index.
 * */
void ccl_devinfo_show_platform_info(CCLPlatform * p, guint idx) {

    /* Platform info variables. */
    gchar * profile, * version, * name, * vendor;

    /* Error location. */
    CCLErr * err = NULL;

    /* Get platform profile. */
    profile = ccl_platform_get_info_string(p, CL_PLATFORM_PROFILE, &err);
    if (err != NULL) {
        ccl_err_clear(&err);
        profile = "Unknown profile";
    }

    /* Get platform version. */
    version = ccl_platform_get_info_string(p, CL_PLATFORM_VERSION, &err);
    if (err != NULL) {
        ccl_err_clear(&err);
        version = "Unknown version";
    }

    /* Get platform name. */
    name = ccl_platform_get_info_string(p, CL_PLATFORM_NAME, &err);
    if (err != NULL) {
        ccl_err_clear(&err);
        name = "Unknown name";
    }

    /* Get platform vendor. */
    vendor = ccl_platform_get_info_string(p, CL_PLATFORM_VENDOR, &err);
    if (err != NULL) {
        ccl_err_clear(&err);
        vendor = "Unknown vendor";
    }

    /*  Send info to defined stream. */
    g_fprintf(CCL_DEVINFO_OUT,
        "\n* Platform #%d: %s (%s)\n               %s, %s\n",
        idx, name, vendor, version, profile);

    /* Bye. */
    return;
}

#define ccl_devinfo_output_device_info(key, value, desc) \
    if (opt_verb) { \
        g_fprintf(CCL_DEVINFO_OUT, \
            "\t\t   Parameter : %s\n" \
            "\t\t Description : %s\n" \
            "\t\t       Value : %s\n\n", \
            key, desc, value); \
    } else { \
        g_fprintf(CCL_DEVINFO_OUT, \
            "        %-36.36s | %s\n", \
            key, value); \
    }

/**
 * Show all available device information.
 *
 * @param[in] d Device wrapper object.
 * */
void ccl_devinfo_show_device_info_all(CCLDevice * d) {

    /* Parameter value and size. */
    CCLWrapperInfo * param_value;

    /* Parameter value string. */
    gchar param_value_str[CCL_DEVINFO_MAXINFOLEN];

    /* Error reporting object. */
    CCLErr * err = NULL;

    /* Cycle through all supported device information names. */
    for (gint k = 0; k < ccl_devquery_info_map_size; k++) {

        /* Get the device information value and size. */
        param_value = ccl_device_get_info(
            d, ccl_devquery_info_map[k].device_info, &err);

        /* Check for errors. */
        if (err == NULL) {

            /* If no error, show current parameter value... */
            ccl_devinfo_output_device_info(
                ccl_devquery_info_map[k].param_name,
                ccl_devquery_info_map[k].format(
                    param_value, param_value_str,
                    CCL_DEVINFO_MAXINFOLEN,
                    ccl_devquery_info_map[k].units),
                ccl_devquery_info_map[k].description);

        } else {

            /* ...otherwise clear error... */
            ccl_err_clear(&err);
            if (opt_nfound) {
                /* ...and show that parameter is not available, if user
                 * requested so. */
                ccl_devinfo_output_device_info(
                    ccl_devquery_info_map[k].param_name,
                    CCL_DEVINFO_NA,
                    ccl_devquery_info_map[k].description);
            }
        }
    }
}

/**
 * Show user specified device information.
 *
 * @param[in] d Device wrapper object.
 * */
void ccl_devinfo_show_device_info_custom(CCLDevice * d) {

    /* A row of the device info_map. */
    const CCLDevQueryMap * info_row;

    /* Parameter value and size. */
    CCLWrapperInfo * param_value;

    /* Parameter value string. */
    gchar param_value_str[CCL_DEVINFO_MAXINFOLEN];

    /* Error reporting object. */
    CCLErr * err = NULL;

    /* Custom parameter name in proper format. */
    gchar * custom_param_name;

    /* Index of next row of the device info_map. */
    gint idx;

    /* Cycle through all user specified parameter substrings. */
    for (guint i = 0; opt_custom[i] != NULL; i++) {

        /* Set index of next row to zero. */
        idx = 0;

        /* Put info name in proper format. */
        custom_param_name = ccl_devquery_get_prefix_final(opt_custom[i]);

        /* Get next row (the first one). */
        info_row = ccl_devquery_match(custom_param_name, &idx);

        /* Keep getting rows until we reach the end of the device info_map. */
        while (info_row != NULL) {

            /* Get parameter value for current info_map row. */
            param_value = ccl_device_get_info(d, info_row->device_info, &err);

            /* Check for errors. */
            if (err == NULL) {

                /* If no error, show current parameter value... */
                ccl_devinfo_output_device_info(
                    info_row->param_name,
                    info_row->format(
                        param_value, param_value_str,
                        CCL_DEVINFO_MAXINFOLEN,
                        info_row->units),
                    info_row->description);

            } else {

                /* ...otherwise clear error... */
                ccl_err_clear(&err);
                if (opt_nfound) {
                    /* ...and show that parameter is not available, if user
                    * requested so. */
                    ccl_devinfo_output_device_info(
                        info_row->param_name,
                        CCL_DEVINFO_NA,
                        info_row->description);
                }
            }

            /* Get next row. */
            info_row = ccl_devquery_match(custom_param_name, &idx);

        }

        /* Free the proper format custom parameter name. */
        g_free(custom_param_name);
    }
}

/**
 * Show basic device information.
 *
 * @param[in] d Device wrapper object.
 * */
void ccl_devinfo_show_device_info_basic(CCLDevice * d) {

    /* Index of info map row. */
    int info_row_index;

    /* A row of the device info_map. */
    const CCLDevQueryMap * info_row;

    /* Parameter value and size. */
    CCLWrapperInfo * param_value;

    /* Parameter value string. */
    gchar param_value_str[CCL_DEVINFO_MAXINFOLEN];

    /* Error reporting object. */
    CCLErr * err = NULL;

    /* Cycle through the pre-defined basic information array. */
    for (guint i = 0; basic_info[i] != NULL; i++) {

        /* Get index of info map row. */
        info_row_index = ccl_devquery_get_index(basic_info[i]);

        /* Check that its a valid index, otherwise we have a
         * programming error, so it is better to abort. */
        g_assert_cmpint(info_row_index, >=, 0);
        g_assert_cmpint(info_row_index, <, ccl_devquery_info_map_size);

        /* Get next row. */
        info_row = &ccl_devquery_info_map[info_row_index];

        /* Get parameter value for current info_map row. */
        param_value = ccl_device_get_info(d, info_row->device_info, &err);

        /* Check for errors. */
        if (err == NULL) {

            /* If no error, show current parameter value... */
            ccl_devinfo_output_device_info(
                info_row->param_name,
                info_row->format(
                    param_value, param_value_str,
                    CCL_DEVINFO_MAXINFOLEN,
                    info_row->units),
                info_row->description);

        } else {

            /* ...otherwise clear error... */
            ccl_err_clear(&err);
            if (opt_nfound) {
                /* ...and show that parameter is not available, if user
                 * requested so. */
                ccl_devinfo_output_device_info(
                    info_row->param_name,
                    CCL_DEVINFO_NA,
                    info_row->description);
            }
        }
    }
}

/**
 * Device info main program function.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Vector of command line arguments.
 * @return ::CCL_SUCCESS if program returns with no error, or another
 * ::CCLErrorCode value otherwise.
 */
int main(int argc, char * argv[]) {

    /* Error object. */
    CCLErr * err = NULL;

    /* List of platform wrapper objects. */
    CCLPlatforms * platforms = NULL;

    /* List of device wrapper objects. */
    CCLDevSelDevices devices = NULL;

    /* Current platform and device. */
    CCLPlatform * p;
    CCLDevice * d;

    /* Number of devices in platform. */
    guint num_devs;

    /* Device information value object. */
    CCLWrapperInfo * info_value = NULL;

    /* Device name. */
    gchar * dev_name;

    /* Program return status. */
    gint status;

    /* Parse command line options. */
    ccl_devinfo_args_parse(argc, argv, &err);
    ccl_if_err_goto(err, error_handler);

    /* If version was requested, output version and exit. */
    if (version) {
        ccl_common_version_print("ccl_devinfo");
        exit(0);
    }

    /* Check if user requested a list of known information parameters. */
    if (opt_list) {

        /*Yes, user requested list, present it. */

        g_fprintf(CCL_DEVINFO_OUT, "\nKnown information parameters:\n\n");
        for (gint i = 0; i < ccl_devquery_info_map_size; i++) {
            if (opt_verb) {
                g_fprintf(CCL_DEVINFO_OUT,
                    "\t%s\n\t\t%s.\n\n",
                    ccl_devquery_info_map[i].param_name,
                    ccl_devquery_info_map[i].description);
            } else {
                g_fprintf(CCL_DEVINFO_OUT,
                    "\t%s\n",
                    ccl_devquery_info_map[i].param_name);
            }
        }
        g_fprintf(CCL_DEVINFO_OUT, "\n");

    } else {

        /* User didn't request list, proceed as normal query. */

        /* Ignore platforms and focus only on number of devices in system? */
        if (no_platf) {

            /* Ignore platform, continue device-wise. */

            /* Get all devices in the system. */
            devices = ccl_devsel_devices_new(&err);
            ccl_if_err_goto(err, error_handler);

            /* Cycle through devices. */
            for (guint j = 0; j < devices->len; j++) {

                /* Get out if this device is not to be queried. */
                if ((opt_dev != G_MAXUINT) && (j != opt_dev))
                    continue;

                /* Get current device. */
                d = (CCLDevice *) devices->pdata[j];

                /* Get device name. */
                info_value = ccl_device_get_info(d, CL_DEVICE_NAME, &err);
                ccl_if_err_goto(err, error_handler);

                dev_name = (gchar *) info_value->value;

                /* Show device information. */
                g_fprintf(CCL_DEVINFO_OUT,
                    "\n    [ Device #%d: %s ]\n\n",
                    j, dev_name);
                ccl_devinfo_show_device_info(d);

            }
            g_fprintf(CCL_DEVINFO_OUT, "\n");

        } else {

            /* Do not ignore platforms, continue platform-wise. */

            /* Get list of platform wrapper objects. */
            platforms = ccl_platforms_new(&err);
            ccl_if_err_goto(err, error_handler);

            /* Cycle through platforms. */
            for (guint i = 0; i < ccl_platforms_count(platforms); i++) {

                /* Get out if this platform is not to be queried. */
                if ((opt_platf != G_MAXUINT) && (i != opt_platf))
                    continue;

                /* Get current platform. */
                p = ccl_platforms_get(platforms, i);

                /* Show platform information. */
                ccl_devinfo_show_platform_info(p, i);

                /* Get number of devices. */
                num_devs = ccl_platform_get_num_devices(p, &err);

                /* Is this a platform without devices? */
                if ((err) && (err->domain == CCL_OCL_ERROR) &&
                        (err->code == CL_DEVICE_NOT_FOUND)) {

                    /* Clear "device not found" error. */
                    ccl_err_clear(&err);

                    /* Inform about non-existing devices. */
                    g_fprintf(CCL_DEVINFO_OUT,
                        "\n    [ No devices found ]\n\n");

                    /* Skip this platform. */
                    continue;
                }
                ccl_if_err_goto(err, error_handler);

                /* Cycle through devices. */
                for (guint j = 0; j < num_devs; j++) {

                    /* Get out if this device is not to be queried. */
                    if ((opt_dev != G_MAXUINT) && (j != opt_dev))
                        continue;

                    /* Get current device. */
                    d = ccl_platform_get_device(p, j, &err);
                    ccl_if_err_goto(err, error_handler);

                    /* Get device name. */
                    info_value = ccl_device_get_info(d, CL_DEVICE_NAME, &err);
                    ccl_if_err_goto(err, error_handler);

                    dev_name = (gchar *) info_value->value;

                    /* Show device information. */
                    g_fprintf(CCL_DEVINFO_OUT,
                        "\n    [ Device #%d: %s ]\n\n",
                        j, dev_name);
                    ccl_devinfo_show_device_info(d);

                }
                g_fprintf(CCL_DEVINFO_OUT, "\n");
            }
        }
    }

    /* If we got here, everything is OK. */
    g_assert(err == NULL);
    status = CCL_SUCCESS;
    goto cleanup;

error_handler:

    /* If we got here there was an error, verify that it is so. */
    g_assert(err != NULL);
    g_fprintf(stderr, "%s\n", err->message);
    status = (err->domain == CCL_ERROR) ? err->code : CCL_ERROR_OTHER;
    g_error_free(err);

cleanup:

    /* Free stuff! */
    if (platforms) ccl_platforms_destroy(platforms);
    if (devices) ccl_devsel_devices_destroy(devices);
    g_strfreev(opt_custom);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

    /* Return status. */
    return status;
}
