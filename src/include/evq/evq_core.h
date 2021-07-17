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
#include <evq/evq_event.h>
#include <evq/evq_handle.h>

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
