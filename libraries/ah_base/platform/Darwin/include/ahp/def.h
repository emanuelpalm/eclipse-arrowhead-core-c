// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_DEF_H
#define AHP_DEF_H

#include <stdint.h>

#define AH_DARWIN

#define ahp_inline __attribute((unused)) static inline
#define ahp_thread_local __thread

typedef int ahp_err_t;
typedef struct iovec ahp_buf_t;
typedef struct ahp_loop ahp_loop_t;
typedef struct ahp_loop_evt ahp_loop_evt_t;
typedef uint64_t ahp_time_t;

#endif
