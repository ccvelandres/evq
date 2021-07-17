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
        EVQ_SE_CTRL_MSG         = (1 << 0), /** General event for evq messages */
        EVQ_SE_ROUTE_MSG        = (1 << 1), /** Notify stack with message for routing  */
        EVQ_SE_HDL_MSG_RX       = (1 << 2), /** Handle event for rx messages */
        EVQ_SE_HDL_MSG_RESPONSE = (1 << 3), /** Handle event for response messages */
        EVQ_SE_HDL_MSG_TX_ERROR = (1 << 4), /** Handle event for tx error messages */
    } evq_stack_events_t;

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

    typedef enum
    {
        EVQ_SE_UNINITIALIZED,
        EVQ_SE_MSG_HDL_REGISTER,
        EVQ_SE_MSG_HDL_UNREGISTER,
        EVQ_SE_MSG_EVT_POST,
    } evq_se_msg_type_t;

    typedef struct
    {
        evq_handle_priv_t      *handle;
    } evq_se_msg_hdl_t;

    typedef struct
    {
        evq_id_t           srcId;
        evq_id_t           evtId;
    } evq_se_msg_evt_t;

    typedef struct
    {
        evq_se_msg_type_t msgType;
        union
        {
            evq_se_msg_hdl_t hdl_register;
            evq_se_msg_hdl_t hdl_unregister;
            evq_se_msg_evt_t evt_post;
        } msg;
    } evq_core_message_t;
    
    /**
     * Structure containing all control data for evq instance
    */
    typedef struct
    {
        evq_core_state_t state;
    #if defined(EVQ_RTOS_SUPPORT)
        evq_mutex_t mutex;
    #endif
        evq_stream_t      *se_stream;
        evq_egroup_t       se_egroup;
        uint16_t           handleCount;
        evq_handle_priv_t *handles[EVQ_MAX_HANDLES];
    } evq_context_t;

   /**
     * @brief Sends se messages for processing
     *
     * Sends @p cMsg by copy.
     *
     * @code
     * void foo() {
     *     evq_core_message_t msg;
     *     // Populate msg here...
     *     evq_se_send_msg(&msg);
     * }
     * @endcode
     *
     * @param[in] cMsg msg to send
     * @return EVQ_ERROR_NONE on success
     */
    evq_status_t evq_se_send_msg(evq_core_message_t *cMsg);

#ifdef __cplusplus
}
#endif
#endif
