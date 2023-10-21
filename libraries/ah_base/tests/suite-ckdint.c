// SPDX-License-Identifier: EPL-2.0

#include <ah/ckdint.h>
#include <ah/unit.h>

AH_UNIT_SUITE(ckdint)
{
    AH_UNIT_TEST("ah_ckd_add() should produce expected results.")
    {
#define GEN_CASE_ADD_I_(type, max, min)                        \
    AH_UNIT_CASE(#type)                                        \
    {                                                          \
        type res;                                              \
                                                               \
        AH_UNIT_EQ_BOOL(false, ah_ckd_add(&res, max - 1, 1));  \
        AH_UNIT_EQ_INT(max, res);                              \
                                                               \
        AH_UNIT_EQ_BOOL(false, ah_ckd_add(&res, min + 1, -1)); \
        AH_UNIT_EQ_INT(min, res);                              \
                                                               \
        AH_UNIT_EQ_BOOL(true, ah_ckd_add(&res, max, 1));       \
        AH_UNIT_EQ_BOOL(true, ah_ckd_add(&res, min, -1));      \
    }                                                          \
    (void) 0

        GEN_CASE_ADD_I_(int8_t, INT8_MAX, INT8_MIN);
        GEN_CASE_ADD_I_(int16_t, INT16_MAX, INT16_MIN);
        GEN_CASE_ADD_I_(int32_t, INT32_MAX, INT32_MIN);
        GEN_CASE_ADD_I_(int64_t, INT64_MAX, INT64_MIN);

#undef GEN_CASE_ADD_I_

#define GEN_CASE_ADD_U_(type, max)                              \
    AH_UNIT_CASE(#type)                                         \
    {                                                           \
        type res;                                               \
                                                                \
        AH_UNIT_EQ_BOOL(false, ah_ckd_add(&res, max - 1u, 1u)); \
        AH_UNIT_EQ_INT(max, res);                               \
                                                                \
        AH_UNIT_EQ_BOOL(true, ah_ckd_add(&res, max, 1u));       \
    }                                                           \
    (void) 0

        GEN_CASE_ADD_U_(uint8_t, UINT8_MAX);
        GEN_CASE_ADD_U_(uint16_t, UINT16_MAX);
        GEN_CASE_ADD_U_(uint32_t, UINT32_MAX);
        GEN_CASE_ADD_U_(uint64_t, UINT64_MAX);

#undef GEN_CASE_ADD_U_
    }

    AH_UNIT_TEST("ah_ckd_mul() should produce expected results.")
    {
#define GEN_CASE_MUL_I_(type, max, min)                       \
    AH_UNIT_CASE(#type)                                       \
    {                                                         \
        type res;                                             \
                                                              \
        AH_UNIT_EQ_BOOL(false, ah_ckd_mul(&res, max / 2, 2)); \
        AH_UNIT_EQ_INT(max - 1, res);                         \
                                                              \
        AH_UNIT_EQ_BOOL(false, ah_ckd_mul(&res, min / 2, 2)); \
        AH_UNIT_EQ_INT(min, res);                             \
                                                              \
        AH_UNIT_EQ_BOOL(true, ah_ckd_mul(&res, max, 2));      \
        AH_UNIT_EQ_BOOL(true, ah_ckd_mul(&res, min, 2));      \
    }                                                         \
    (void) 0

        GEN_CASE_MUL_I_(int8_t, INT8_MAX, INT8_MIN);
        GEN_CASE_MUL_I_(int16_t, INT16_MAX, INT16_MIN);
        GEN_CASE_MUL_I_(int32_t, INT32_MAX, INT32_MIN);
        GEN_CASE_MUL_I_(int64_t, INT64_MAX, INT64_MIN);

#undef GEN_CASE_MUL_I_

#define GEN_CASE_MUL_U_(type, max)                              \
    AH_UNIT_CASE(#type)                                         \
    {                                                           \
        type res;                                               \
                                                                \
        AH_UNIT_EQ_BOOL(false, ah_ckd_mul(&res, max / 2u, 2u)); \
        AH_UNIT_EQ_INT(max - 1u, res);                          \
                                                                \
        AH_UNIT_EQ_BOOL(true, ah_ckd_mul(&res, max, 2u));       \
    }                                                           \
    (void) 0

        GEN_CASE_MUL_U_(uint8_t, UINT8_MAX);
        GEN_CASE_MUL_U_(uint16_t, UINT16_MAX);
        GEN_CASE_MUL_U_(uint32_t, UINT32_MAX);
        GEN_CASE_MUL_U_(uint64_t, UINT64_MAX);

#undef GEN_CASE_MUL_U_
    }

    AH_UNIT_TEST("ah_ckd_sub() should produce expected results.")
    {
#define GEN_CASE_SUB_I_(type, max, min)                     \
    AH_UNIT_CASE(#type)                                     \
    {                                                       \
        type res;                                           \
                                                            \
        AH_UNIT_EQ_BOOL(false, ah_ckd_sub(&res, max, max)); \
        AH_UNIT_EQ_INT(0, res);                             \
                                                            \
        AH_UNIT_EQ_BOOL(false, ah_ckd_sub(&res, min, min)); \
        AH_UNIT_EQ_INT(0, res);                             \
                                                            \
        AH_UNIT_EQ_BOOL(true, ah_ckd_sub(&res, max, -1));   \
        AH_UNIT_EQ_BOOL(true, ah_ckd_sub(&res, min, 1));    \
    }                                                       \
    (void) 0

        GEN_CASE_SUB_I_(int8_t, INT8_MAX, INT8_MIN);
        GEN_CASE_SUB_I_(int16_t, INT16_MAX, INT16_MIN);
        GEN_CASE_SUB_I_(int32_t, INT32_MAX, INT32_MIN);
        GEN_CASE_SUB_I_(int64_t, INT64_MAX, INT64_MIN);

#undef GEN_CASE_SUB_I_

#define GEN_CASE_SUB_U_(type, max)                              \
    AH_UNIT_CASE(#type)                                         \
    {                                                           \
        type res;                                               \
                                                                \
        AH_UNIT_EQ_BOOL(false, ah_ckd_sub(&res, max, max));     \
        AH_UNIT_EQ_INT(0u, res);                                \
                                                                \
        AH_UNIT_EQ_BOOL(true, ah_ckd_sub(&res, max - 1u, max)); \
    }                                                           \
    (void) 0

        GEN_CASE_SUB_U_(uint8_t, UINT8_MAX);
        GEN_CASE_SUB_U_(uint16_t, UINT16_MAX);
        GEN_CASE_SUB_U_(uint32_t, UINT32_MAX);
        GEN_CASE_SUB_U_(uint64_t, UINT64_MAX);

#undef GEN_CASE_MUL_U_
    }
}
