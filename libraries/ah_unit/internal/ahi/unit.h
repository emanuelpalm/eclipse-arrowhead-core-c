// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_UNIT_H
#define AHI_UNIT_H

#include <ah/err.h>
#include <stdbool.h>
#include <stddef.h>

#define AHI_UNIT_SUITE_IDENT(name) ahi_usr_unit_suite_##name
#define AHI_UNIT_LOC               ((ahi_unit_loc_t) { ._file = __FILE__, ._line = __LINE__ })

typedef struct ahi_unit ahi_unit_t;
typedef struct ahi_unit_loc ahi_unit_loc_t;

enum ahi_unit_state {
    AHI_STATE_STOPPED,
    AHI_STATE_RUNNING,
    AHI_STATE_FAILING,
    AHI_STATE_SKIPPING,
};

struct ahi_unit {
    enum ahi_unit_state _suite_state;
    enum ahi_unit_state _test_state;
    enum ahi_unit_state _case_state;

    const char* _suite_msg;
    const char* _test_msg;
    char _case_msg[80u];

    int _test_idx;
    int _case_idx;

    int _suite_run_count;
    int _suite_fail_count;
    int _suite_skip_count;

    int _test_run_count;
    int _test_fail_count;
    int _test_skip_count;
};

struct ahi_unit_loc {
    const char* _file;
    int _line;
};

void ahi_unit_init(ahi_unit_t* u, int argc, const char** argv);
void ahi_unit_exit(ahi_unit_t* u);

void ahi_unit_run_suite(ahi_unit_t* u, const char* msg, void (*suite)(ahi_unit_t*));
bool ahi_unit_run_test(ahi_unit_t* u, const char* msg);
bool ahi_unit_run_case(ahi_unit_t* u, const char* fmt, ...);

bool ahi_unit_eq_bool(ahi_unit_t* u, ahi_unit_loc_t l, bool a, bool b);
bool ahi_unit_eq_err(ahi_unit_t* u, ahi_unit_loc_t l, ah_err_t a, ah_err_t b);
bool ahi_unit_eq_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b);
bool ahi_unit_eq_mem(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b, size_t sz);
bool ahi_unit_eq_ptr(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b);
bool ahi_unit_eq_str(ahi_unit_t* u, ahi_unit_loc_t l, const char* a, const char* b);
bool ahi_unit_eq_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);
bool ahi_unit_eq_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);
bool ahi_unit_ge_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);
bool ahi_unit_ge_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);
bool ahi_unit_gt_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b);
bool ahi_unit_gt_uhex(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);
bool ahi_unit_lt_int(ahi_unit_t* u, ahi_unit_loc_t l, intmax_t a, intmax_t b);
bool ahi_unit_ne_mem(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b, size_t sz);
bool ahi_unit_ne_ptr(ahi_unit_t* u, ahi_unit_loc_t l, void* a, void* b);
bool ahi_unit_ne_uint(ahi_unit_t* u, ahi_unit_loc_t l, uintmax_t a, uintmax_t b);

void ahi_unit_skip(ahi_unit_t* u, ahi_unit_loc_t l, const char* fmt, ...);

#endif
