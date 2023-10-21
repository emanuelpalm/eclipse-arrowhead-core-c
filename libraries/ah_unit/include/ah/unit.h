// SPDX-License-Identifier: EPL-2.0

#ifndef AH_UNIT_H
#define AH_UNIT_H

#include <ahi/unit.h>

#define AH_UNIT_INIT(argc, argv)                                             \
    ahi_unit_t _ahi_usr_test_ctx_, *_ahi_usr_test_ctx = &_ahi_usr_test_ctx_; \
    ahi_unit_init(_ahi_usr_test_ctx, (argc), (argv))

#define AH_UNIT_RUN_SUITE(name) \
    AH_UNIT_SUITE(name);        \
    ahi_unit_run_suite(_ahi_usr_test_ctx, (#name), AHI_UNIT_SUITE_IDENT(name))

#define AH_UNIT_EXIT() ahi_unit_exit(_ahi_usr_test_ctx)

#define AH_UNIT_SUITE(name) \
    void AHI_UNIT_SUITE_IDENT(name)(ah_unused ahi_unit_t * _ahi_usr_test_ctx)

#define AH_UNIT_TEST(msg) \
    if (ahi_unit_run_test(_ahi_usr_test_ctx, (msg)))

#define AH_UNIT_CASE(...) \
    if (ahi_unit_run_case(_ahi_usr_test_ctx, __VA_ARGS__))

#define AH_UNIT_EQ_BOOL(expected, actual) \
    ahi_unit_eq_bool(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_ERR(expected, actual) \
    ahi_unit_eq_err(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_INT(expected, actual) \
    ahi_unit_eq_int(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_PTR(expected, actual) \
    ahi_unit_eq_ptr(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_STR(expected, actual) \
    ahi_unit_eq_str(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_UHEX(expected, actual) \
    ahi_unit_eq_uhex(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_EQ_UINT(expected, actual) \
    ahi_unit_eq_uint(_ahi_usr_test_ctx, AHI_UNIT_LOC, (expected), (actual))

#define AH_UNIT_GE_UHEX(lesser, greater) \
    ahi_unit_ge_uhex(_ahi_usr_test_ctx, AHI_UNIT_LOC, (lesser), (greater))

#define AH_UNIT_GT_UHEX(lesser, greater) \
    ahi_unit_gt_uhex(_ahi_usr_test_ctx, AHI_UNIT_LOC, (lesser), (greater))

#define AH_UNIT_NE_PTR(unexpected, actual) \
    ahi_unit_ne_ptr(_ahi_usr_test_ctx, AHI_UNIT_LOC, (unexpected), (actual))

#define AH_UNIT_NE_UINT(unexpected, actual) \
    ahi_unit_ne_uint(_ahi_usr_test_ctx, AHI_UNIT_LOC, (unexpected), (actual))

#define AH_UNIT_SKIP(...) \
    ahi_unit_skip(_ahi_usr_test_ctx, AHI_UNIT_LOC, __VA_ARGS__)

#endif
