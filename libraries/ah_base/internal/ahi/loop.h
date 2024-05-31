// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_LOOP_H
#define AHI_LOOP_H

#include <ah/def.h>
#include <ahp/loop.h>

#define AHI_LOOP_STATE_INITIAL     0
#define AHI_LOOP_STATE_RUNNING     1
#define AHI_LOOP_STATE_STOPPING    2
#define AHI_LOOP_STATE_STOPPED     3
#define AHI_LOOP_STATE_TERMINATING 4
#define AHI_LOOP_STATE_TERMINATED  5

ah_err_t ahi_loop_alloc_evt(ah_loop_t* l, ahi_loop_evt_t** evt);
void ahi_loop_free_evt(ah_loop_t* l, ahi_loop_evt_t* evt);

struct ahi_loop_evt {
    ahp_loop_t _p;
    void *_subject;
};

#endif
