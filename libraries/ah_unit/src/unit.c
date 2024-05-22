// SPDX-License-Identifier: EPL-2.0

#include <ah/err.h>
#include <ah/unit.h>
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ahi_format_mem(void* src, size_t src_sz, char* dst, size_t dst_sz);
static void ahi_report_err(ahi_unit_t* u, ahi_unit_loc_t l, const char* fmt, ...);

void ahi_unit_init(ahi_unit_t* u, int argc, const char** argv)
{
    *u = (ahi_unit_t) {
        ._suite_state = AHI_STATE_STOPPED,
        ._test_state = AHI_STATE_STOPPED,
        ._case_state = AHI_STATE_STOPPED,
    };

    (void) argc;
    (void) argv;
}

void ahi_unit_exit(ahi_unit_t* u)
{
    if (u->_suite_run_count != 0u) {
        int suite_pass_count = u->_suite_run_count;
        suite_pass_count -= u->_suite_skip_count;
        suite_pass_count -= u->_suite_fail_count;

        printf("Passed %d/%d suites", suite_pass_count, u->_suite_run_count);

        if (u->_suite_skip_count != 0u) {
            printf(" (skipped %d)", u->_suite_skip_count);
        }

        int test_pass_count = u->_test_run_count;
        test_pass_count -= u->_test_skip_count;
        test_pass_count -= u->_test_fail_count;

        printf(" and %d/%d tests", test_pass_count, u->_test_run_count);

        if (u->_test_skip_count != 0u) {
            printf(" (skipped %d)", u->_test_skip_count);
        }

        puts(".");
    }

    if (u->_suite_fail_count != 0u) {
        fprintf(stderr, "Failed %d suites and %d tests.\n", u->_suite_fail_count,
            u->_test_fail_count);
    }

    exit(u->_suite_fail_count == 0u ? EXIT_SUCCESS : EXIT_FAILURE);
}

void ahi_unit_run_suite(ahi_unit_t* u, const char* msg, void (*suite)(ahi_unit_t*))
{
    u->_suite_run_count += 1u;
    u->_suite_msg = msg;
    u->_suite_state = AHI_STATE_RUNNING;

    u->_test_idx = 0u;
    u->_test_msg = NULL;
    u->_test_state = AHI_STATE_STOPPED;

    u->_case_idx = 0u;
    u->_case_msg[0u] = '\0';
    u->_case_state = AHI_STATE_STOPPED;

    suite(u);
}

bool ahi_unit_run_test(ahi_unit_t* u, const char* msg)
{
    u->_test_run_count += 1u;
    u->_test_idx += 1u;
    u->_test_msg = msg;
    u->_test_state = AHI_STATE_RUNNING;

    u->_case_idx = 0u;
    u->_case_msg[0u] = '\0';
    u->_case_state = AHI_STATE_STOPPED;

    return true;
}

bool ahi_unit_run_case(ahi_unit_t* u, const char* fmt, ...)
{
    u->_case_idx += 1u;

    const size_t sz = sizeof(u->_case_msg);

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        int res = vsnprintf(u->_case_msg, sz, fmt, args);
        va_end(args);
        if (res < 0) {
            (void) strncpy(u->_case_msg, fmt, sz);
        }
    }
    else {
        (void) strncpy(u->_case_msg, "--", sz);
    }

    if (u->_case_state == AHI_STATE_STOPPED) {
        u->_case_state = AHI_STATE_RUNNING;
    }
    else {
        u->_test_run_count += 1u;
    }

    return true;
}

bool ahi_unit_eq_bool(ahi_unit_t* u, ahi_unit_loc_t l, bool a, bool b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected `%s`; but got `%s`.\n\n";
    ahi_report_err(u, l, fmt, a ? "true" : "false", b ? "true" : "false");

    return false;
}

bool ahi_unit_eq_err(ahi_unit_t* u, ahi_unit_loc_t l, ah_err_t a, ah_err_t b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected `%s`; but got `%s`.\n\n";
    ahi_report_err(u, l, fmt, ah_err_get_s(a), ah_err_get_s(b));

    return false;
}

bool ahi_unit_eq_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIiMAX "; received: %" PRIiMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_eq_mem(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b, size_t sz)
{
    if (memcmp(a, b, sz) == 0) {
        return true;
    }

    char buf_a[128u];
    ahi_format_mem(a, sz, buf_a, sizeof(buf_a));

    char buf_b[128u];
    ahi_format_mem(b, sz, buf_b, sizeof(buf_b));

    const char* fmt = "Expected: %s\n\t\tReceived: %s\n\n";
    ahi_report_err(u, l, fmt, buf_a, buf_b);

    return false;
}

bool ahi_unit_eq_ptr(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected %p; received %p.\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_eq_str(ahi_unit_t* u, ahi_unit_loc_t l, const char* a, const char* b)
{
    if (strcmp(a, b) == 0) {
        return true;
    }

    const char* fmt = "Expected: %s\n\t\tReceived: %s\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_eq_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected: %#" PRIxMAX "; received: %#" PRIxMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_eq_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a == b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIuMAX "; received: %" PRIuMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_ge_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a >= b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIxMAX " >= %" PRIxMAX "\n\n";
    ahi_report_err(u, l, fmt, a, a);

    return false;
}

bool ahi_unit_ge_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a >= b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIuMAX " >= %" PRIuMAX "\n\n";
    ahi_report_err(u, l, fmt, a, a);

    return false;
}

bool ahi_unit_gt_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b)
{
    if (a > b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIiMAX " > %" PRIiMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_gt_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a > b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIxMAX " > %" PRIxMAX "\n\n";
    ahi_report_err(u, l, fmt, b, a);

    return false;
}

bool ahi_unit_lt_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b)
{
    if (a < b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIiMAX " < %" PRIiMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_ne_mem(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b, size_t sz)
{
    if (memcmp(a, b, sz) != 0) {
        return true;
    }

    char buf[256u];
    ahi_format_mem(a, sz, buf, sizeof(buf));

    const char* fmt = "Unexpected: %s\n\n";
    ahi_report_err(u, l, fmt, buf);

    return false;
}

bool ahi_unit_ne_ptr(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b)
{
    if (a != b) {
        return true;
    }

    const char* fmt = "Expected: %p != %p\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

bool ahi_unit_ne_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b)
{
    if (a != b) {
        return true;
    }

    const char* fmt = "Expected: %" PRIuMAX " != %" PRIuMAX "\n\n";
    ahi_report_err(u, l, fmt, a, b);

    return false;
}

void ahi_unit_skip(ahi_unit_t* u, ahi_unit_loc_t l, const char* fmt, ...)
{
    if (u->_suite_state != AHI_STATE_STOPPED) {
        if (u->_test_state == AHI_STATE_STOPPED) {
            (void) printf("SUITE SKIPPED: [%s]\n\t", u->_suite_msg);
        }
        else {
            (void) printf("TEST SKIPPED: [%s] %s\n\t", u->_suite_msg, u->_test_msg);
        }
        if (u->_case_state != AHI_STATE_STOPPED) {
            (void) printf("CASE[%d] %s ", u->_case_idx, u->_case_msg);
        }
        printf("%s:%d\n\t\t", l._file, l._line);
    }

    va_list args;
    va_start(args, fmt);
    (void) vprintf(fmt, args);
    va_end(args);

    fputs("\n\n", stdout);

    if (u->_case_state == AHI_STATE_RUNNING) {
        u->_case_state = AHI_STATE_SKIPPING;
        if (u->_test_state == AHI_STATE_SKIPPING) {
            u->_test_skip_count += 1u;
        }
    }

    if (u->_test_state == AHI_STATE_RUNNING) {
        u->_test_state = AHI_STATE_SKIPPING;
        u->_test_skip_count += 1u;
    }

    if (u->_suite_state == AHI_STATE_RUNNING) {
        u->_suite_state = AHI_STATE_SKIPPING;
        if (u->_test_state == AHI_STATE_STOPPED) {
            u->_suite_skip_count += 1u;
        }
    }
}

static void ahi_format_mem(void* src, size_t src_sz, char* dst, size_t dst_sz)
{
    assert(dst_sz > 0u);

    static const char* DIGITS = "0123456789ABCDEF";

    size_t dst_i = 0u;
    for (size_t src_i = 0u; src_i < src_sz; src_i++) {
        const char d0 = DIGITS[(((uint8_t*) src)[src_i] >> 0u) & 0xF];
        if (dst_i >= dst_sz) {
            break;
        }
        dst[dst_i++] = d0;

        const char d1 = DIGITS[(((uint8_t*) src)[src_i] >> 4u) & 0xF];
        if (dst_i >= dst_sz) {
            break;
        }
        dst[dst_i++] = d1;
    }

    if (dst_i < dst_sz) {
        dst[dst_i] = '\0';
    }
    else {
        dst[dst_sz - 1u] = '\0';
    }
}

static void ahi_report_err(ahi_unit_t* u, ahi_unit_loc_t l, const char* fmt, ...)
{
    if (u->_suite_state != AHI_STATE_STOPPED && u->_test_state != AHI_STATE_STOPPED) {
        (void) fprintf(stderr, "TEST FAILED: [%s] %s\n\t", u->_suite_msg, u->_test_msg);

        if (u->_case_state != AHI_STATE_STOPPED) {
            (void) fprintf(stderr, "CASE[%d] %s ", u->_case_idx, u->_case_msg);
        }

        fprintf(stderr, "%s:%d\n\t\t", l._file, l._line);
    }

    va_list args;
    va_start(args, fmt);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);

    fputs("\n\n", stdout);

    if (u->_case_state == AHI_STATE_RUNNING) {
        u->_case_state = AHI_STATE_FAILING;
        if (u->_test_state == AHI_STATE_FAILING) {
            u->_test_fail_count += 1u;
        }
    }

    if (u->_test_state == AHI_STATE_RUNNING) {
        u->_test_state = AHI_STATE_FAILING;
        u->_test_fail_count += 1u;
    }

    if (u->_suite_state == AHI_STATE_RUNNING) {
        u->_suite_state = AHI_STATE_FAILING;
        u->_suite_fail_count += 1u;
    }
}
