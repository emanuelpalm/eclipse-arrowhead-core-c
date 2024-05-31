// SPDX-License-Identifier: EPL-2.0

#include <ah/loop.h>
#include <assert.h>

static void ahi_loop_cancel_evt(void* evt);
static void ahi_loop_term(ah_loop_t* l);

ah_err_t ah_loop_init(ah_loop_t* l)
{
    if (l == NULL) {
        return AH_EINVAL;
    }

    *l = (ah_loop_t) { 0u };

    ahp_err_t err = ahp_loop_init(l);
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
        err = ahp_loop_run_until(l, t);
        if (err != AHP_OK) {
            break;
        }
    } while (l->_state == AHI_LOOP_STATE_RUNNING && (t == NULL || ah_time_is_before(l->_now, *t)));

    if (l->_state == AHI_LOOP_STATE_TERMINATING) {
        ahi_loop_term(l);
    }
    else {
        l->_state = AHI_LOOP_STATE_STOPPED;
    }

    return err;
}

ah_err_t ah_loop_term(ah_loop_t* l)
{
    if (l == NULL) {
        return AH_EINVAL;
    }

    ah_err_t err;

    switch (l->_state) {
    case AHI_LOOP_STATE_INITIAL:
#ifndef NDEBUG
        *l = (ah_loop_t) { 0 };
#endif
        l->_state = AHI_LOOP_STATE_TERMINATED;
        err = AH_OK;
        break;

    case AHI_LOOP_STATE_RUNNING:
        l->_state = AHI_LOOP_STATE_TERMINATING;
        err = AH_OK;
        break;

    case AHI_LOOP_STATE_STOPPING:
    case AHI_LOOP_STATE_STOPPED:
        ahi_loop_term(l);
        err = AH_OK;
        break;

    default:
        err = AH_ESTATE;
        break;
    }

    return err;
}

ah_err_t ahi_loop_alloc_evt(ah_loop_t* l, ahi_loop_evt_t** evt)
{
    assert(l != NULL);
    assert(evt != NULL);

    if (ah_loop_is_term(l)) {
        return AH_ECANCELED;
    }

    ahi_loop_evt_t* free_evt = ah_slab_alloc(&l->_evt_slab);
    if (free_evt == NULL) {
        return AH_ENOMEM;
    }

    *evt = free_evt;

    return AH_OK;
}

void ahi_loop_free_evt(ah_loop_t* l, ahi_loop_evt_t* evt)
{
    assert(l != NULL);
    assert(evt != NULL);

    ah_slab_free(&l->_evt_slab, evt);
}

static void ahi_loop_term(ah_loop_t* l)
{
    ah_slab_term(&l->_evt_slab, ahi_loop_cancel_evt);
    ahp_loop_term(l);

#ifndef NDEBUG
    *l = (ah_loop_t) { 0 };
#endif

    l->_state = AHI_LOOP_STATE_TERMINATED;
}

static void ahi_loop_cancel_evt(void* evt)
{
    (void) evt; // TODO: Implement
}
