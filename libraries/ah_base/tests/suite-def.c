// SPDX-License-Identifier: EPL-2.0

#include <ah/unit.h>
#include <stdio.h>

#define TO_STRING_LITERAL0_(v) #v
#define TO_STRING_LITERAL_(v)  TO_STRING_LITERAL0_(v)

AH_UNIT_SUITE(def)
{
    AH_UNIT_TEST("AH_BASE_VERSION contains expected parts.")
    {
        char major[8u];
        char minor[8u];
        char patch[8u];
        char platform[64u];
        char commit[256u];

        int res = sscanf(AH_BASE_VERSION, "%7[^.].%7[^.].%7[^+]+%63[^.].%255s",
            major, minor, patch, platform, commit);

        if (AH_UNIT_EQ_INT(5, res)) {
            AH_UNIT_EQ_STR(TO_STRING_LITERAL_(AH_BASE_VERSION_MAJOR), major);
            AH_UNIT_EQ_STR(TO_STRING_LITERAL_(AH_BASE_VERSION_MINOR), minor);
            AH_UNIT_EQ_STR(TO_STRING_LITERAL_(AH_BASE_VERSION_PATCH), patch);
            AH_UNIT_EQ_STR(AH_BASE_PLATFORM, platform);
            AH_UNIT_EQ_STR(AH_BASE_COMMIT, commit);
        }
    }
}
