// SPDX-License-Identifier: EPL-2.0

#include "ah/json.h"

#include <ah/err.h>
#include <ah/math.h>

ah_extern ah_err_t ah_json_num_parse_int32(const char* src, size_t src_length, int32_t* dst)
{
    if ((src == NULL && src_length != 0u) || dst == NULL) {
        return AH_EINVAL;
    }

    ah_err_t err;

    bool has_integer_digits = false;
    bool has_nonzero_fraction = false;
    bool has_fraction_digits = false;
    bool has_exponent_digits = false;
    bool has_exponent_minus_sign = false;
    bool has_nonzero_exponent = false;

    int32_t sign = 1;
    int32_t integer = 0;
    uint8_t exponent = 0;

    if (src_length == 0u) {
        return AH_ESYNTAX;
    }

    if (src[0u] == '-') {
        sign = -1;

        src = &src[1u];
        src_length -= 1u;

        if (src_length == 0) {
            return AH_ESYNTAX;
        }
    }

    if (src[0u] == '0') {
        src = &src[1u];
        src_length -= 1u;

        has_integer_digits = true;

        if (src_length == 0u) {
            goto handle_end;
        }

        if (src[0u] == '.') {
            goto handle_fraction;
        }

        if (src[0u] == 'E' || src[0u] == 'e') {
            goto handle_exponent;
        }

        return AH_ESYNTAX;
    }

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        err = ah_mul_int32(integer, 10, &integer);
        if (err != AH_ENONE) {
            return err;
        }

        err = ah_add_int32(integer, ch - '0', &integer);
        if (err != AH_ENONE) {
            if (integer == 2147483640 && ch == '8' && sign == -1) {
                integer = -2147483648;
                sign = 1;
            }
            else {
                return err;
            }
        }

        has_integer_digits = true;

        src = &src[1u];
        src_length -= 1u;
    }

    if (!has_integer_digits) {
        return AH_ESYNTAX;
    }

    if (src_length == 0u) {
        goto handle_end;
    }

    if (src[0u] != '.') {
        goto expect_exponent;
    }

handle_fraction:
    src = &src[1u];
    src_length -= 1u;

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        has_fraction_digits = true;

        if (ch != '0') {
            has_nonzero_fraction = true;
        }

        src = &src[1u];
        src_length -= 1u;
    }

    if (!has_fraction_digits) {
        return AH_ESYNTAX;
    }

    if (src_length == 0u) {
        goto handle_end;
    }

expect_exponent:
    if (src[0u] != 'E' && src[0u] != 'e') {
        return AH_ESYNTAX;
    }

handle_exponent:
    src = &src[1u];
    src_length -= 1u;

    if (src[0u] == '+') {
        src = &src[1u];
        src_length -= 1u;
    }
    else if (src[0u] == '-') {
        has_exponent_minus_sign = true;

        src = &src[1u];
        src_length -= 1u;
    }

    while (src_length != 0u && src[0u] == '0') {
        has_exponent_digits = true;

        src = &src[1u];
        src_length -= 1u;
    }

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        err = ah_mul_uint8(exponent, 10, &exponent);
        if (err != AH_ENONE) {
            return err;
        }

        err = ah_add_uint8(exponent, ch - '0', &exponent);
        if (err != AH_ENONE) {
            return err;
        }

        has_exponent_digits = true;
        has_nonzero_exponent = true;

        src = &src[1u];
        src_length -= 1u;
    }

    if (src_length != 0u || !has_exponent_digits) {
        return AH_ESYNTAX;
    }

    if (has_nonzero_fraction && has_nonzero_exponent) {
        return AH_EOPNOTSUPP;
    }

    for (; exponent != 0u; exponent -= 1u) {
        if (has_exponent_minus_sign) {
            err = ah_div_int32(integer, 10, &integer);
        }
        else {
            err = ah_mul_int32(integer, 10, &integer);
        }
        if (err != AH_ENONE) {
            return err;
        }
    }

handle_end:
    *dst = sign * integer;

    return has_nonzero_fraction ? AH_EDOM : AH_ENONE;
}

ah_extern bool ah_json_num_validate(const char* src, size_t src_length)
{
    if (src == NULL || src_length == 0u) {
        return false;
    }

    bool has_integer_digits = false;
    bool has_fraction_digits = false;
    bool has_exponent_digits = false;

    if (src[0u] == '-') {

        src = &src[1u];
        src_length -= 1u;

        if (src_length == 0) {
            return false;
        }
    }

    if (src[0u] == '0') {
        src = &src[1u];
        src_length -= 1u;

        has_integer_digits = true;

        if (src_length == 0u) {
            return true;
        }

        if (src[0u] == '.') {
            goto handle_fraction;
        }

        if (src[0u] == 'E' || src[0u] == 'e') {
            goto handle_exponent;
        }

        return false;
    }

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        has_integer_digits = true;

        src = &src[1u];
        src_length -= 1u;
    }

    if (!has_integer_digits) {
        return false;
    }

    if (src_length == 0u) {
        return true;
    }

    if (src[0u] != '.') {
        goto expect_exponent;
    }

handle_fraction:
    src = &src[1u];
    src_length -= 1u;

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        has_fraction_digits = true;

        src = &src[1u];
        src_length -= 1u;
    }

    if (!has_fraction_digits) {
        return false;
    }

    if (src_length == 0u) {
        return true;
    }

expect_exponent:
    if (src[0u] != 'E' && src[0u] != 'e') {
        return false;
    }

handle_exponent:
    src = &src[1u];
    src_length -= 1u;

    if (src[0u] == '+' || src[0u] == '-') {
        src = &src[1u];
        src_length -= 1u;
    }

    while (src_length != 0u) {
        char ch = src[0u];
        if (ch < '0' || ch > '9') {
            break;
        }

        has_exponent_digits = true;

        src = &src[1u];
        src_length -= 1u;
    }

    if (src_length != 0u || !has_exponent_digits) {
        return false;
    }

    return true;
}
