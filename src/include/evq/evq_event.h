// SPDX-License-Identifier: MIT

#ifndef __EVQ_EVENT_H__
#define __EVQ_EVENT_H__

/**
 * @file evq/evq_event.h
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_core.h>
#include <evq/evq_types.h>

    typedef struct
    {
        evq_id_t srcId; // Source Id of the event
        evq_id_t evtId; // Event Id
    } evq_event_priv_t;

    /** @brief Subscribe to specific events matching id */
    evq_status_t evq_subscribe(evq_handle_t handle, evq_id_t evtId);
    /** @brief Subscribe to events in array (Removes previously subsrcibed events) */
    evq_status_t evq_subscribe_a(evq_handle_t handle, const evq_id_t *evtId, uint32_t cnt);
    /** @brief Post an event */
    evq_status_t evq_post_event(evq_handle_t handle, evq_id_t evtId);
    /** @brief Poll for event (Only if event handler is null) */
    evq_status_t evq_poll_event(evq_handle_t handle, evq_id_t *evtId);

#ifdef __cplusplus
}
#endif
#endif
