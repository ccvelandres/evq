#include <stddef.h>

#include <evq/evq_log.h>
#include <evq/evq_event.h>
#include <evq/evq_core_p.h>

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
            st                       = EVQ_ERROR_NONE;
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
    memset(privHandle->eventList, 0 , sizeof(privHandle->eventList));

    // add all event ids
    for (uint32_t i = 0; i < cnt; i)
    {
        privHandle->eventList[i] = evtId[i];
    }

    return st;
}

evq_status_t evq_post_event(evq_handle_t handle, evq_id_t evtId)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = (evq_handle_priv_t *)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(0 != evtId, "evtId cannot be 0");

    {
        // send a core message
        evq_core_message_t cMsg = {
            .msgType = EVQ_SE_EVT_CTRL,
            .msg.evt = {
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

evq_status_t evq_poll_event(evq_handle_t handle, evq_id_t *evtId)
{
    evq_status_t       st         = EVQ_ERROR_NONE;
    evq_handle_priv_t *privHandle = (evq_handle_priv_t *)handle;

    EVQ_ASSERT(NULL != handle, "handle argument is null");
    EVQ_ASSERT(0 != evtId, "evtId cannot be 0");

    return st;
}
