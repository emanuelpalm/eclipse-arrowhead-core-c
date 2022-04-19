// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#ifndef AH_INTERNAL_POSIX_BUF_H_
#define AH_INTERNAL_POSIX_BUF_H_

#include "../../defs.h"

#define AH_I_BUF_PLATFORM_FIELDS                                                                                       \
    uint8_t* _octets;                                                                                                  \
    size_t _size;

struct iovec;

ah_extern ah_err_t ah_i_bufvec_from_iovec(ah_bufvec_t* bufvec, struct iovec* iov, int iovcnt);
ah_extern ah_err_t ah_i_bufvec_into_iovec(ah_bufvec_t* bufvec, struct iovec** iov, int* iovcnt);

#endif
