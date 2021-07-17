#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include <evq_core_p.h>
#include <evq/evq_port.h>
#include <evq/evq_log.h>
#include <evq/evq_message.h>

evq_status_t evq_message_allocate(evq_message_t *msg, uint32_t len)
{
    evq_status_t        ret        = EVQ_ERROR_NMEM;
    int                 dataLength = 0;
    evq_message_priv_t *priv       = NULL;

    if (NULL == msg)
    {
        return EVQ_ERROR_NARG;
    }

    // Datalen of pointersize is guaranteed
    dataLength = (len > sizeof(void *)) ? len : 0;
    priv = (evq_message_priv_t *)evq_malloc(sizeof(evq_message_priv_t) + dataLength);
    if (NULL == priv)
    {
        EVQ_LOG_TRACE("Could not allocate memory");
        ret = EVQ_ERROR_NMEM;
    }
    else
    {
        priv->len = len;
        ret       = EVQ_ERROR_NONE;
    }

    *msg = (evq_message_t)priv;
    return ret;
}

evq_status_t evq_message_destroy(evq_message_t msg)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    evq_free(priv);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_get_src_id(evq_message_t msg, evq_id_t *srcId)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != srcId, "Null argument not allowed");
    *srcId = priv->srcId;
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_get_dst_id(evq_message_t msg, evq_id_t *dstId)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != dstId, "Null argument not allowed");
    *dstId = priv->dstId;
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_get_msg_id(evq_message_t msg, evq_id_t *msgId)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != msgId, "Null argument not allowed");
    *msgId = priv->msgId;
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_get_data(evq_message_t msg, uint8_t **ptr)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != ptr, "Null argument not allowed");
    *ptr = &priv->data[0];
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_pop_data(evq_message_t msg, uint8_t **data, uint32_t *len)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != data, "Null argument not allowed");
    EVQ_ASSERT(NULL != len, "Null argument not allowed");
    *data = &priv->data[0];
    *len  = priv->len;
    return EVQ_ERROR_NONE;
}

evq_status_t evq_message_put_data(evq_message_t msg, const void *data, uint32_t len)
{
    evq_message_priv_t *priv = (evq_message_priv_t *)msg;
    EVQ_ASSERT(NULL != msg, "Null argument not allowed");
    EVQ_ASSERT(NULL != data, "Null argument not allowed");
    EVQ_ASSERT(0 > len, "Zero length data not allowed");
    EVQ_ASSERT(priv->len < len, "length exceeds message size");
    (void)memcpy(&priv->data[0], data, len);
    return EVQ_ERROR_NONE;
}