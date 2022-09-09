// SPDX-License-Identifier: EPL-2.0

#include "ah/json.h"

#include "ah_i_json_lib_version.h"

ah_extern const char* ah_json_lib_version_str(void)
{
    return AH_I_JSON_LIB_VERSION_STR;
}

ah_extern unsigned short ah_json_lib_version_major(void)
{
    return AH_I_JSON_LIB_VERSION_MAJOR;
}

ah_extern unsigned short ah_json_lib_version_minor(void)
{
    return AH_I_JSON_LIB_VERSION_MINOR;
}

ah_extern unsigned short ah_json_lib_version_patch(void)
{
    return AH_I_JSON_LIB_VERSION_PATCH;
}
