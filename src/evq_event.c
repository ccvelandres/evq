#include <stddef.h>
#include <string.h>

#include <evq/evq_log.h>
#include <evq/evq_event.h>
#include <evq_core_p.h>

static void evq_priv_event_post(evq_context_t *ctx, const evq_se_msg_evt_t *eMsg)
{
    uint32_t           postCount = 0;
    evq_status_t       st        = EVQ_ERROR_NONE;
    evq_handle_priv_t *hdl       = NULL;

    // iterate through handles and check if they are subscribed
    for (size_t handleIndex = 0; handleIndex < EVQ_MAX_HANDLES; ++handleIndex)
    {
        hdl = ctx->handles[handleIndex];
        if ((NULL == hdl) || (!hdl->isActive) || (NULL == hdl->eventHandler)
            || (eMsg->srcId == hdl->handleId))
        {
            // skip null handles, inactive handles, null event handlers, and srcHandle
            continue;
        }

        // check sub list
        for (size_t subIndex = 0; subIndex < hdl->eventSubCount; ++subIndex)
        {
            if (hdl->eventList[subIndex] == eMsg->evtId)
            {
                // call handle callback on match
                hdl->eventHandler(eMsg->srcId, eMsg->evtId);
                // todo: wake handle based on return
                postCount++;
                break;
            }
        }
    }
    EVQ_LOG_TRACE("Routed event from (0x%4X) with eventId: 0x%4X to %4u handles\n",
                  eMsg->srcId,
                  eMsg->evtId,
                  postCount);
}

evq_status_t evq_event_process(evq_context_t *ctx, const evq_core_message_t *cMsg)
{
    evq_status_t       st  = EVQ_ERROR_NONE;
    evq_handle_priv_t *hdl = NULL;

    switch (cMsg->msgType)
    {
    case EVQ_SE_MSG_EVT_POST:
        (void)evq_priv_event_post(ctx, &cMsg->msg.evt_post);
        break;
    default:
        EVQ_LOG_TRACE("Unhandled message type: %u\n", cMsg->msgType);
        break;
    }

    return st;
}

////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////

evq_status_t evq_subscribe(evq_handle_t handle, evq_id_t evtId)
{
    evq_status_t       st         = EVQ_ERROR_LIST_FULL;
    evq_handle_priv_t *privHandle = (evq_handle_priv_t *)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(0 != evtId, "evtId cannot be 0");

    // add evtId to list
    for (uint32_t i = 0; i < EVQ_MAX_EVENT_SUBSCRIBE_COUNT; ++i)
    {
        if (0 == privHandle->eventList[i])
        {
            privHandle->eventList[i] = evtId;
            privHandle->eventSubCount += 1;
            st = EVQ_ERROR_NONE;
        }
    }

    return st;
}

evq_status_t evq_subscribe_a(evq_handle_t handle, const evq_id_t *evtId, uint32_t cnt)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = (evq_handle_priv_t *)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(NULL != evtId, "evtId cannot be null");
    EVQ_ASSERT(0 != cnt, "cnt cannot be 0");
    EVQ_ASSERT(EVQ_MAX_EVENT_SUBSCRIBE_COUNT > cnt,
               "cnt cannot exceed EVQ_MAX_EVENT_SUBSCRIBE_COUNT");

    // Clear event list
    // (not sure if this is preferred or retain the previous event list)
    memset(privHandle->eventList, 0, sizeof(privHandle->eventList));

    // add all event ids
    for (uint32_t i = 0; i < cnt; ++i)
    {
        privHandle->eventList[i] = evtId[i];
    }

    privHandle->eventSubCount = cnt;
    return st;
}

evq_status_t evq_post_event(evq_handle_t handle, evq_id_t evtId)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = (evq_handle_priv_t *)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");

    {
        // send a core message
        evq_core_message_t cMsg = {
            .msgType = EVQ_SE_MSG_EVT_POST,
            .msg.evt_post = {
                .srcId = privHandle->handleId,
                .evtId = evtId,
            },
        };

        st = evq_se_send_msg(&cMsg);
        if (EVQ_ERROR_NONE != st)
        {
            EVQ_LOG_TRACE("Could not send event post message to core\n");
        }
    }

    return st;
}
