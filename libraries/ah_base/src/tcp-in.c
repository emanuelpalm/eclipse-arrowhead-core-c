// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "tcp-in.h"

#include "ah/assert.h"

ah_tcp_in_t* ah_i_tcp_in_alloc(void)
{
    uint8_t* page = ah_palloc();
    if (page == NULL) {
        return NULL;
    }

    ah_tcp_in_t* in = (void*) page;

    *in = (ah_tcp_in_t) {
        .buf = ah_buf_from(&page[sizeof(ah_tcp_in_t)], AH_PSIZE - sizeof(ah_tcp_in_t)),
        .nread = 0u,
    };

    return in;
}

void ah_i_tcp_in_free(ah_tcp_in_t* in)
{
    ah_assert_if_debug(in != NULL);

    ah_pfree(in);
}
