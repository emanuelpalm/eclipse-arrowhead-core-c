// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "../src/http-iheaders.h"

#include <ah/err.h>
#include <ah/unit.h>
#include <stdlib.h>

static void s_should_add_and_get_headers(ah_unit_t* unit);
static void s_should_add_same_header_name_multiple_times(ah_unit_t* unit);

void test_http_iheaders(ah_unit_t* unit)
{
    s_should_add_and_get_headers(unit);
    s_should_add_same_header_name_multiple_times(unit);
}

static void s_should_add_and_get_headers(ah_unit_t* unit)
{
    ah_err_t err;

    ah_http_iheaders_t headers;
    err = ah_i_http_iheaders_init(&headers, realloc, 2u);
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        return;
    }

    // Add headers.

    err = ah_i_http_iheaders_add(&headers, "host", "192.168.40.40:40404");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    err = ah_i_http_iheaders_add(&headers, "content-type", "application/json");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    // Capacity is 2; this should fail.
    err = ah_i_http_iheaders_add(&headers, "content-length", "16");
    if (!ah_unit_assert_enum_eq(unit, AH_ENOBUFS, err, ah_strerror)) {
        goto term;
    }

    // Get headers.

    bool has_next;
    const char* value;

    value = ah_http_iheaders_get_value(&headers, "Host", &has_next);
    if (!ah_unit_assert(unit, !has_next, "there should only exist one host name/value pair")) {
        goto term;
    }
    if (!ah_unit_assert_str_eq(unit, "192.168.40.40:40404", value)) {
        goto term;
    }

    value = ah_http_iheaders_get_value(&headers, "Content-Type", &has_next);
    if (!ah_unit_assert(unit, !has_next, "there should only exist one host name/value pair")) {
        goto term;
    }
    if (!ah_unit_assert_str_eq(unit, "application/json", value)) {
        goto term;
    }

    // This header should not be present.
    value = ah_http_iheaders_get_value(&headers, "Content-Length", &has_next);
    if (!ah_unit_assert_str_eq(unit, NULL, value)) {
        goto term;
    }

term:
    ah_i_http_iheaders_term(&headers, realloc);
}

static void s_should_add_same_header_name_multiple_times(ah_unit_t* unit)
{
    ah_err_t err;

    ah_http_iheaders_t headers;
    err = ah_i_http_iheaders_init(&headers, realloc, 4u);
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        return;
    }

    // Add headers.

    err = ah_i_http_iheaders_add(&headers, "set-cookie", "munchy");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    err = ah_i_http_iheaders_add(&headers, "SET-CookIe", "crispy");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    err = ah_i_http_iheaders_add(&headers, "Host", "[::1]:12345");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    err = ah_i_http_iheaders_add(&headers, "Set-Cookie", "sweet");
    if (!ah_unit_assert_enum_eq(unit, AH_ENONE, err, ah_strerror)) {
        goto term;
    }

    // Get headers.

    ah_http_iheader_value_iter_t iter = ah_http_iheaders_get_values(&headers, "Set-Cookie");

    const char* value;

    value = ah_http_iheaders_next(&iter);
    if (!ah_unit_assert_str_eq(unit, "munchy", value)) {
        goto term;
    }

    value = ah_http_iheaders_next(&iter);
    if (!ah_unit_assert_str_eq(unit, "crispy", value)) {
        goto term;
    }

    value = ah_http_iheaders_next(&iter);
    if (!ah_unit_assert_str_eq(unit, "sweet", value)) {
        goto term;
    }

    value = ah_http_iheaders_next(&iter);
    if (!ah_unit_assert_str_eq(unit, NULL, value)) {
        goto term;
    }

term:
    ah_i_http_iheaders_term(&headers, realloc);
}
