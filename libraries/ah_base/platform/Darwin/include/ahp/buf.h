// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_BUF_H_
#define AHP_BUF_H_

#include "def.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/uio.h>

#define AHP_BUF_SIZE_MAX SIZE_MAX

ahp_inline int ahp_buf_init(struct iovec* b, uint8_t* base, size_t sz)
{
    if (b == NULL || (base == NULL && sz != 0u)) {
        return EINVAL;
    }

    b->iov_base = base;
    b->iov_len = sz;

    return 0;
}

ahp_inline uint8_t* ahp_buf_get_base(struct iovec* b)
{
    return b != NULL ? b->iov_base : NULL;
}

ahp_inline size_t ahp_buf_get_sz(struct iovec* b)
{
    return b != NULL ? b->iov_len : 0u;
}

#endif
