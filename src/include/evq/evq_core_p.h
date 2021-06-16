// SPDX-License-Identifier: MIT

#ifndef __EVQ_CORE_P_H__
#define __EVQ_CORE_P_H__

/**
 * @brief Private header for evq_core
 * @file evq/evq_core_p.hpp
 * @author Cedric Velandres (ccvelandres@gmail.com)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <evq/evq_types.h>
#include <evq/evq_core.h>
#include <evq/evq_message.h>
#include <evq/evq_stream.h>
#include <evq/evq_handle_p.h>

    typedef enum
    {
        EVQ_CS_UNINITIALIZED,
        EVQ_CS_INITIALIZED,
        EVQ_CS_RUNNING,
        EVQ_CS_SUSPENDED,
        EVQ_CS_SHUTDOWN
    } evq_core_state_t;

    typedef enum
    {
        EVQ_HS_UNINITIALIZED,
        EVQ_HS_IDLE,
        EVQ_HS_WAITING,
        EVQ_HS_RESPONSE,
        EVQ_HS_UNREGISTERED
    } evq_handle_state_t;

    typedef enum
    {
        EVQ_CMT_UNINITIALIZED,
        EVQ_CMT_HANDLE_REGISTER,
        EVQ_CMT_HANDLE_UNREGISTER,
        EVQ_CMT_EVENT_POST,
    } evq_core_message_type_t;

    typedef enum
    {
        EVQ_C_EV_MSG              = (1 << 0),
        EVQ_C_EV_HDL_MSG_TX       = (1 << 1),
        EVQ_C_EV_HDL_MSG_RX       = (1 << 2),
        EVQ_C_EV_HDL_MSG_RESPONSE = (1 << 3),
        EVQ_C_EV_HDL_MSG_TX_ERROR = (1 << 4),
    } evq_core_events_t;

    typedef struct
    {
        evq_handle_state_t  state;
        const char         *handleName;
        evq_id_t            handleId;
        volatile bool       isRegistered;
        volatile bool       isActive;
        evq_egroup_t        egroup;
        evq_stream_t       *rxStream;
        evq_stream_t       *txStream;
        uint32_t            seqNum;
        evq_event_handler_t eventHandler;
        uint32_t            eventSubCount;
        evq_id_t            eventList[EVQ_MAX_EVENT_SUBSCRIBE_COUNT];
    } evq_handle_priv_t;

    typedef struct
    {
        evq_core_message_type_t msgType;
        union
        {
            struct
            {
                evq_handle_priv_t *handle;
            } hdl_reg;
            struct {
                evq_id_t srcId;
                evq_id_t evtId;
            } evt;
        } msg;
    } evq_core_message_t;

#ifdef __cplusplus
}
#endif
#endif
