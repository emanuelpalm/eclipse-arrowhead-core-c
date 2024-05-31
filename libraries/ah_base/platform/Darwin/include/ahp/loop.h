// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_LOOP_H
#define AHP_LOOP_H

#include "def.h"

struct ahp_loop {
    int _kqueue_fd;
    int _kqueue_nchanges;
    struct kevent* _kqueue_changelist;
    struct kevent* _kqueue_eventlist;
};

struct ahp_loop_evt {
    void (*_cb)(struct ahp_loop_evt*, struct kevent*);
    void *_subject;
};

ahp_err_t ahp_loop_init(ahp_loop_t* l);
ahp_err_t ahp_loop_alloc_evt(ahp_loop_t* l, struct ahp_loop_evt** evt);
ahp_err_t ahp_loop_alloc_evt_kev(ahp_loop_t* l, struct ahp_loop_evt** evt, struct kevent** kev);
ahp_err_t ahp_loop_alloc_kev(ahp_loop_t* l, struct kevent** kev);
void ahp_loop_free_evt(ahp_loop_t* l, struct ahp_loop_evt* evt);
ahp_err_t ahp_loop_run_until(ahp_loop_t* l, ahp_time_t* t);
void ahp_loop_term(ahp_loop_t* l);

#endif
