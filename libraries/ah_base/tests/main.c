// SPDX-License-Identifier: EPL-2.0

#include <ah/unit.h>

int main(int argc, const char** argv)
{
    AH_UNIT_INIT(argc, argv);

    AH_UNIT_RUN_SUITE(alloc_align);
    AH_UNIT_RUN_SUITE(alloc_bump);
    AH_UNIT_RUN_SUITE(alloc_page);
    AH_UNIT_RUN_SUITE(alloc_slab);
    AH_UNIT_RUN_SUITE(buf);
    AH_UNIT_RUN_SUITE(ckdint);
    AH_UNIT_RUN_SUITE(def);
    AH_UNIT_RUN_SUITE(err);
    AH_UNIT_RUN_SUITE(time);

    AH_UNIT_EXIT();
}
