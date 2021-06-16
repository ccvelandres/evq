// SPDX-License-Identifier: MIT

#ifndef __EVQ_CORE_H__
#define __EVQ_CORE_H__

/**
 * @file evq/evq_core.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_types.h>
#include <evq/evq_message.h>

    typedef struct
    {
        uint8_t dummy[20];
    } evq_static_handle_t;

    typedef void *evq_handle_t;

    typedef struct
    {
        const char         *handleName;
        evq_id_t            handleId;
        uint32_t            queueSize;
        evq_event_handler_t eventHandler;
    } evq_handle_config_t;

    /** @brief Allocate evq handle */
    evq_status_t evq_handle_register(evq_handle_t *handle, const evq_handle_config_t *config);
    evq_status_t evq_handle_unregister(evq_handle_t *handle);

    /** @brief Send message */
    evq_status_t evq_send(evq_handle_t  handle,
                          evq_id_t      dstId,
                          evq_id_t      messageId,
                          evq_message_t message);

    /** @brief Receive message */
    evq_status_t evq_receive(evq_handle_t handle, evq_message_t *message, uint32_t timeout);

    /** @brief Send message and wait for response */
    evq_status_t evq_send_receive(evq_handle_t   handle,
                                  evq_id_t       dstId,
                                  evq_id_t       messageId,
                                  evq_message_t *message,
                                  uint32_t       timeout);

    /** @brief Subscribe to specific events matching id */
    evq_status_t evq_subscribe(evq_handle_t handle, evq_id_t evtId);
    /** @brief Subscribe to events in array */
    evq_status_t evq_subscribe_a(evq_handle_t handle, const evq_id_t *evtId, uint32_t cnt);
    /** @brief Post an event */
    evq_status_t evq_post_event(evq_handle_t handle, evq_id_t evtId);
    /** @brief Poll for event (Only if event handler is null) */
    evq_status_t evq_poll_event(evq_handle_t handle, evq_id_t *evtId);

    // bool evq_post_async(evq_handle_t handle, evq_addr_t destId, evq_message_t message);
    // bool evq_post_sync(evq_handle_t handle, evq_addr_t destId, evq_message_t message);

    // bool evq_broadcast(evq_handle_t handle, const evq_message_t *message);

    /** @brief Initialize evq stack */
    evq_status_t evq_init();
    /** @brief Shutdown evq stack */
    evq_status_t evq_shutdown();

    /**
     * @brief Main function for the evq system
     * You can either:
     *  - Create a dedicated task that calls this function periodically (with RTOS)
     *  - Call this function periodically in main loop (baremetal)
     */
    void evq_process(void);

#ifdef __cplusplus
}
#endif
#endif
