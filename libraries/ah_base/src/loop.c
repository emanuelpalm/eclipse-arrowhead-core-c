// SPDX-License-Identifier: EPL-2.0

#include <ah/loop.h>

static void ahi_loop_cancel_evt(void* evt);
static void ahi_term(ah_loop_t* loop);

ah_err_t ah_loop_init(ah_loop_t* l)
{
    if (l == NULL) {
        return AH_EINVAL;
    }

    *l = (ah_loop_t) { 0u };

    ahp_err_t err = ahp_loop_init(&l->_p);
    if (err != AHP_OK) {
        return (ah_err_t) err;
    }

    err = ah_slab_init(&l->_evt_slab, sizeof(ahp_loop_evt_t));
    if (err != AH_OK) {
        ah_loop_term(l);
        return err;
    }

    l->_now = ah_time_now();
    l->_state = AHI_LOOP_STATE_INITIAL;

    return AH_OK;
}

ah_err_t ah_loop_run_until(ah_loop_t* l, ah_time_t* t)
{
    if (l == NULL) {
        return AH_EINVAL;
    }
    if (l->_state != AHI_LOOP_STATE_INITIAL && l->_state != AHI_LOOP_STATE_STOPPED) {
        return AH_ESTATE;
    }
    l->_state = AHI_LOOP_STATE_RUNNING;

    ah_err_t err;

    do {
        err = ahp_loop_run_until(&l->_p, t);
        if (err != AHP_OK) {
            break;
        }
    } while (l->_state == AHI_LOOP_STATE_RUNNING && (t == NULL || ah_time_is_before(l->_now, *t)));

    if (l->_state == AHI_LOOP_STATE_TERMINATING) {
        ahi_term(l);
    }
    else {
        l->_state = AHI_LOOP_STATE_STOPPED;
    }

    return err;
}

static void ahi_term(ah_loop_t* loop)
{
    ah_assert_if_debug(loop != NULL);

    ahi_slab_term(&loop->_evt_slab, ahi_loop_cancel_evt);
    ahi_loop_term(loop);

#ifndef NDEBUG
    *loop = (ah_loop_t) { 0 };
#endif

    loop->_state = AHI_LOOP_STATE_TERMINATED;
}

static void ahi_loop_cancel_evt(void* evt)
{
    ahi_loop_evt_call_as_canceled(evt);
}

ah_err_t ah_loop_stop(ah_loop_t* loop)
{
    if (loop == NULL) {
        return AH_EINVAL;
    }
    if (loop->_state != AHI_LOOP_STATE_RUNNING) {
        return AH_ESTATE;
    }
    loop->_state = AHI_LOOP_STATE_STOPPING;
    return AH_ENONE;
}

ah_err_t ah_loop_term(ah_loop_t* loop)
{
    if (loop == NULL) {
        return AH_EINVAL;
    }

    ah_err_t err;

    switch (loop->_state) {
    case AHI_LOOP_STATE_INITIAL:
#ifndef NDEBUG
        *loop = (ah_loop_t) { 0 };
#endif
        loop->_state = AHI_LOOP_STATE_TERMINATED;
        err = AH_ENONE;
        break;

    case AHI_LOOP_STATE_RUNNING:
        loop->_state = AHI_LOOP_STATE_TERMINATING;
        err = AH_ENONE;
        break;

    case AHI_LOOP_STATE_STOPPING:
    case AHI_LOOP_STATE_STOPPED:
        ahi_term(loop);
        err = AH_ENONE;
        break;

    default:
        err = AH_ESTATE;
        break;
    }

    return err;
}

bool ahi_loop_try_set_pending_err(ah_loop_t* loop, ah_err_t err)
{
    if (ah_loop_is_term(loop) || (loop->_pending_err != AH_ENONE && loop->_pending_err != err)) {
        return false;
    }
    loop->_pending_err = err;
    return true;
}

ah_err_t ahi_loop_get_pending_err(ah_loop_t* loop)
{
    ah_assert_if_debug(loop != NULL);

    if (loop->_pending_err != AH_ENONE) {
        ah_err_t err = loop->_pending_err;
        loop->_pending_err = AH_ENONE;
        return err;
    }

    return AH_ENONE;
}

ah_err_t ahi_loop_evt_alloc(ah_loop_t* loop, ahi_loop_evt_t** evt)
{
    ah_assert_if_debug(loop != NULL);
    ah_assert_if_debug(evt != NULL);

    if (ah_loop_is_term(loop)) {
        return AH_ECANCELED;
    }

    ahi_loop_evt_t* free_evt = ahi_slab_alloc(&loop->_evt_slab);
    if (free_evt == NULL) {
        return AH_ENOMEM;
    }

#if AH_USE_IOCP
    free_evt->_overlapped = (OVERLAPPED) { 0u };
#endif

    *evt = free_evt;

    return AH_ENONE;
}

void ahi_loop_evt_dealloc(ah_loop_t* loop, ahi_loop_evt_t* evt)
{
    ah_assert_if_debug(loop != NULL);
    ah_assert_if_debug(evt != NULL);

    ahi_slab_free(&loop->_evt_slab, evt);
}
