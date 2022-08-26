// SPDX-License-Identifier: EPL-2.0

#include "ah/lib.h"
#include "ah/unit.h"

#include <stdio.h>
#include <stdlib.h>

void test_buf(ah_unit_t* unit);
void test_collections_list(ah_unit_t* unit);
void test_collections_ring(ah_unit_t* unit);
void test_ip(ah_unit_t* unit);
void test_math(ah_unit_t* unit);
void test_sock(ah_unit_t* unit);
void test_task(ah_unit_t* unit);
void test_tcp(ah_unit_t* unit);
void test_time(ah_unit_t* unit);
void test_udp(ah_unit_t* unit);
void test_utf8(ah_unit_t* unit);

int main(void)
{
    (void) printf(
        "Arrowhead Base C Library Unit Tests\n"
        "- Source Commit: %s\n"
        "- Platform:      %s\n",
        ah_lib_commit_str(), ah_lib_platform_str());

    struct ah_unit unit = { 0 };

    test_buf(&unit);
    test_collections_list(&unit);
    test_collections_ring(&unit);
    test_ip(&unit);
    test_math(&unit);
    test_sock(&unit);
    test_task(&unit);
    test_tcp(&unit);
    test_time(&unit);
    test_udp(&unit);
    test_utf8(&unit);

    ah_unit_print_results(&unit);

    return unit.fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
