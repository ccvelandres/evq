#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <evq/evq_types.h>
#include <evq/evq_core.h>
#include <evq/evq_log.h>
#include <evq/evq_port.h>
#include <evq/evq_stream.h>
#include <evq/evq_config.h>
#include <evq/evq_core_p.h>

#include <evq/evq_handle.h>
#include <evq/evq_handle_p.h>

#include <evq/evq_event.h>
#include <evq/evq_event_p.h>
/**
 * @file src/evq_core.c
 */

////////////////////////////////////////////////////////////////////
// Macro
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Type declarations
////////////////////////////////////////////////////////////////////

// static_assert(sizeof(evq_handle_priv_t) != sizeof(evq_static_handle_t),
//               "Mismatched static buffer size for evq_handle_t");
// static_assert(EVQ_MAX_HANDLES < UINT16_MAX, "EVQ_MAX_HANDLES cannot exceed UINT16_MAX");

////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////

evq_context_t g_context = {
    .state       = EVQ_CS_UNINITIALIZED,
    .mutex       = NULL,
    .se_stream   = NULL,
    .se_egroup   = NULL,
    .handleCount = 0,
    .handles     = {NULL},
};

////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////

static evq_status_t evq_se_handle_ctrl_msg(evq_context_t *ctx)
{
    evq_status_t       st = 0;
    evq_core_message_t cMsg;

    while (EVQ_ERROR_NONE == (st = evq_stream_pop(ctx->se_stream, &cMsg)))
    {
        if(cMsg.msgType & EVQ_SE_MSG_HDL) evq_handle_process(ctx, &cMsg);
        if(cMsg.msgType & EVQ_SE_MSG_EVT) evq_event_process(ctx, &cMsg);
    }

    return st;
}

static evq_status_t evq_se_handle_route_msg(evq_context_t *ctx)
{
    evq_status_t       st        = EVQ_ERROR_NONE;
    evq_handle_priv_t *srcHandle = NULL;
    evq_handle_priv_t *dstHandle = NULL;
    evq_message_t      seMsg     = NULL;

    // scan handle array for tx streams with content
    for (size_t handleIndex = 0; handleIndex < EVQ_MAX_HANDLES; ++handleIndex)
    {
        srcHandle = ctx->handles[handleIndex];
        if ((NULL == srcHandle) || (!srcHandle->isActive))
        {
            // skip null handles, inactive handles
            continue;
        }

        // pop from tx stream
        st = evq_stream_pop(srcHandle->txStream, &seMsg);
        if (EVQ_ERROR_NONE != st)
        {
            // stream is empty
            continue;
        }

        dstHandle = evq_priv_lookup_handle_with_id(ctx, seMsg->dstId);
        if (NULL == dstHandle)
        {
            // destination not found
            EVQ_LOG_WARNING("dst(%x) not found, dropping message\n", seMsg->dstId);
            (void)evq_message_destroy(seMsg);
            continue;
        }

        // found destination handle
        st = evq_stream_push(dstHandle->rxStream, &seMsg);
        if (EVQ_ERROR_NONE != st)
        {
            // could not send message
            EVQ_LOG_WARNING("Could not push message to dst(%x)\n", dstHandle->handleId);
            (void)evq_message_destroy(seMsg);
            continue;
        }

        st = evq_egroup_set(dstHandle->egroup, EVQ_SE_HDL_MSG_RX, EVQ_CORE_TIMEOUT);
        if (EVQ_ERROR_NONE != st)
        {
            // Could not notify destination, dont drop since message is already
            // in destination rx stream
            EVQ_LOG_TRACE("Could not notify dstHandle(%X)\n", dstHandle->handleId);
        }

        EVQ_LOG_TRACE("Routed message (0x%X)->(0x%X) MSG: 0x%X SEQ: 0x%X\n",
                      seMsg->srcId,
                      seMsg->dstId,
                      seMsg->msgId,
                      seMsg->seqId);
    }

    return st;
}

////////////////////////////////////////////////////////////////////
// Stack Functions
////////////////////////////////////////////////////////////////////

evq_status_t evq_se_send_msg(evq_core_message_t *cMsg)
{
    evq_status_t   st  = EVQ_ERROR_NONE;
    evq_context_t *ctx = &g_context;

    if (EVQ_ERROR_NONE == evq_mutex_lock(ctx->mutex, EVQ_CORE_TIMEOUT))
    {
        st = evq_stream_push(ctx->se_stream, cMsg);
        if (EVQ_ERROR_NONE == st)
        {
            // notify on success
            (void)evq_egroup_set(ctx->se_egroup, EVQ_SE_CTRL_MSG, EVQ_CORE_TIMEOUT);
        }
        evq_mutex_unlock(ctx->mutex);
    }
    return st;
}

////////////////////////////////////////////////////////////////////
// Public Function Definitions
////////////////////////////////////////////////////////////////////

evq_status_t evq_init()
{
    evq_status_t   st  = EVQ_ERROR_NONE;
    evq_context_t *ctx = &g_context;

    if (EVQ_CS_UNINITIALIZED != ctx->state)
    {
        EVQ_LOG_TRACE("evq state is not EVQ_UNINITIALIZED\n");
        return EVQ_ERROR_NONE;
    }

    st = evq_mutex_create(&ctx->mutex);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_ERROR("Could not create mutex\n");
        goto cleanup;
    }

    st = evq_egroup_create(&ctx->se_egroup);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_ERROR("Could not create egroup\n");
        goto cleanup;
    }

    st = evq_stream_create(&ctx->se_stream, sizeof(evq_core_message_t), EVQ_CORE_QUEUE_SIZE);
    if (EVQ_ERROR_NONE != st)
    {
        EVQ_LOG_ERROR("Could not create core stream\n");
        goto cleanup;
    }

cleanup:
    if (EVQ_ERROR_NONE == st)
    {
        ctx->state = EVQ_CS_INITIALIZED;
    }
    else
    {
        if (NULL != ctx->se_stream) (void)evq_stream_destroy(ctx->se_stream);
        if (NULL != ctx->se_egroup) (void)evq_egroup_destroy(ctx->se_egroup);
        if (NULL != ctx->mutex) (void)evq_mutex_destroy(ctx->mutex);
    }
    return EVQ_ERROR_NONE;
}

evq_status_t evq_shutdown()
{
    evq_status_t   st  = EVQ_ERROR_NONE;
    evq_context_t *ctx = &g_context;

    if (NULL != ctx->se_stream) (void)evq_stream_destroy(ctx->se_stream);
    if (NULL != ctx->se_egroup) (void)evq_egroup_destroy(ctx->se_egroup);
    if (NULL != ctx->mutex) (void)evq_mutex_destroy(ctx->mutex);
    return st;
}

void evq_process()
{
    evq_status_t   st        = EVQ_ERROR_NONE;
    evq_context_t *ctx       = &g_context;
    uint32_t       eventBits = 0;

    st = evq_egroup_wait(ctx->se_egroup, 0xFFFF, &eventBits, false, EVQ_CORE_TIMEOUT);
    if ((EVQ_ERROR_NONE != st) && (EVQ_ERROR_TIMEOUT != st))
    {
        EVQ_LOG_TRACE("Error waiting for events: %u\n", st);
    }
    else
    {
        if (eventBits & EVQ_SE_CTRL_MSG) evq_se_handle_ctrl_msg(ctx);
        if (eventBits & EVQ_SE_ROUTE_MSG) evq_se_handle_route_msg(ctx);
    }
}

evq_status_t evq_send(evq_handle_t handle, evq_id_t dstId, evq_id_t messageId, evq_message_t message)
{
    evq_status_t        st          = EVQ_ERROR_NONE;
    evq_handle_priv_t  *privHandle  = (evq_handle_priv_t *)handle;
    evq_message_priv_t *privMessage = (evq_message_priv_t *)message;
    evq_message_t       seMsg       = privMessage;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(NULL != message, "message argument is null");

    seMsg->srcId = privHandle->handleId;
    seMsg->dstId = dstId;
    seMsg->msgId = messageId;
    seMsg->seqId = privHandle->seqNum++;

    EVQ_LOG_TRACE("[%s] Sending message to %X\n", privHandle->handleName, seMsg->dstId);
    st = evq_stream_push(privHandle->txStream, &seMsg);
    if (EVQ_ERROR_NONE == st)
    {
        // notify core on send
        st = evq_egroup_set(g_context.se_egroup, EVQ_SE_ROUTE_MSG, EVQ_CORE_TIMEOUT);
    }
    return st;
}

evq_status_t evq_receive(evq_handle_t handle, evq_message_t *message, uint32_t timeout)
{
    evq_status_t       st            = EVQ_ERROR_NONE;
    uint32_t           remainingTime = timeout;
    evq_handle_priv_t *privHandle    = (evq_handle_priv_t *)handle;
    uint32_t           eventBits     = 0;
    evq_message_t      seMsg         = NULL;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(NULL != message, "message argument is null");

    // set state
    privHandle->state = EVQ_HS_WAITING;
    do
    {
        st = evq_stream_pop(privHandle->rxStream, &seMsg);
        if ((EVQ_ERROR_STREAM_EMPTY == st) && (remainingTime > 0))
        {
            /** Wait for handle rx event. Core process can push to stream while
             * waiting, else this wait times out
             */
            (void)evq_egroup_wait(privHandle->egroup, EVQ_SE_HDL_MSG_RX, &eventBits, true, remainingTime);
            (void)eventBits; // don't really care about the event bits
            // this ensures wait only happens once
            remainingTime = 0;
        }
        else
        {
            break;
        }
    } while (EVQ_ERROR_NONE != st);

    if (EVQ_ERROR_NONE == st)
    {
        EVQ_LOG_TRACE("[%s] Received message from %X\n", privHandle->handleName, seMsg->srcId);
        *message = seMsg;
    }
    else
    {
        // convert stream empty to timeout error, else return the error
        st = (st == EVQ_ERROR_STREAM_EMPTY) ? EVQ_ERROR_TIMEOUT : st;
    }

    privHandle->state = EVQ_HS_IDLE;
    return st;
}

#if defined(EVQ_RTOS_SUPPORT)
evq_status_t evq_send_receive(evq_handle_t   handle,
                              evq_id_t       dstId,
                              evq_id_t       messageId,
                              evq_message_t *message,
                              uint32_t       timeout)
{
    evq_status_t        st          = EVQ_ERROR_NONE;
    evq_handle_priv_t  *privHandle  = (evq_handle_priv_t *)handle;
    evq_message_priv_t *privMessage = *(evq_message_priv_t **)message;
    evq_message_t       seMsg       = privMessage;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(NULL != message, "message argument is null");

    EVQ_ASSERT(0, "Function not yet implemented");

    (void)st;
    (void)privHandle;
    (void)privMessage;
    (void)seMsg;

    return st;
}
#endif
