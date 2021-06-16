// SPDX-License-Identifier: MIT

#ifndef __EVQ_MESSAGE_H__
#define __EVQ_MESSAGE_H__

/**
 * @file evq/evq_message.h
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

    typedef enum
    {
        EVQ_MSG_TYPE_DIRECT,
        EVQ_MSG_TYPE_SYS_EVENT,
        EVQ_MSG_TYPE_USER_EVENT
    } evq_message_type_t;

    typedef enum
    {
        EVQ_SYS_EVENT_START,
        EVQ_SYS_EVENT_SHUTDOWN,
        EVQ_SYS_EVENT_SUSPEND,
    } evq_sys_event_t;

    typedef uint32_t evq_user_event_t;

    typedef struct zEVQ_MSG_PRIV
    {
        evq_id_t srcId;                // Source ID
        evq_id_t dstId;                // Destination ID
        evq_id_t msgId;                // Message ID
        evq_id_t seqId;                // Sequence ID
        uint32_t len;                  // Length of Data
        uint8_t  data[sizeof(void *)]; // Data (hold minimum of pointer size)
    } evq_message_priv_t;

    typedef evq_message_priv_t *evq_message_t;

    evq_status_t evq_message_allocate(evq_message_t *msg, uint32_t len);
    evq_status_t evq_message_allocate_put(evq_message_t *msg, const void *data, uint32_t len);
    evq_status_t evq_message_destroy(evq_message_t msg);

    evq_status_t evq_message_get_src_id(evq_message_t msg, evq_id_t *srcId);
    evq_status_t evq_message_get_dst_id(evq_message_t msg, evq_id_t *dstId);
    evq_status_t evq_message_get_msg_id(evq_message_t msg, evq_id_t *msgId);
    evq_status_t evq_message_get_data(evq_message_t msg, uint8_t **ptr);

    evq_status_t evq_message_pop_data(evq_message_t msg, uint8_t **data, uint32_t *len);
    evq_status_t evq_message_put_data(evq_message_t msg, const void *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif
