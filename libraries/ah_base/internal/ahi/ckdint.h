// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_CKDINT_H
#define AHI_CKDINT_H

#define ahi_ckd_add(res, a, b) ((_Bool) __builtin_add_overflow((a), (b), (res)))
#define ahi_ckd_mul(res, a, b) ((_Bool) __builtin_mul_overflow((a), (b), (res)))
#define ahi_ckd_sub(res, a, b) ((_Bool) __builtin_sub_overflow((a), (b), (res)))

#endif
