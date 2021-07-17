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
#include <evq/evq_types.h>

    /**
     * @brief Function type for event handlers
     * @param[in] srcId sourceId of posting event
     * @param[in] eventId eventId of the event
     * @return true to wakeup waiting handle
     *         false do nothing
    */
    typedef bool (*evq_event_handler_t)(evq_id_t srcId, evq_id_t eventId);

    /** @brief Subscribe to specific events matching id */
    evq_status_t evq_subscribe(evq_handle_t handle, evq_id_t evtId);
    /** @brief Subscribe to events in array (Removes previously subsrcibed events) */
    evq_status_t evq_subscribe_a(evq_handle_t handle, const evq_id_t *evtId, uint32_t cnt);
    /** @brief Post an event */
    evq_status_t evq_post_event(evq_handle_t handle, evq_id_t evtId);

#ifdef __cplusplus
}
#endif
#endif
