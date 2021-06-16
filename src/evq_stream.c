#include <stddef.h>
#include <string.h>

#include <evq/evq_stream.h>
#include <evq/evq_port.h>
#include <evq/evq_log.h>

static uint32_t evq_stream_get_space(const evq_stream_t *stream, uint32_t head, uint32_t tail)
{
    uint32_t count = stream->len + tail - head - 1;

    count = (count >= stream->len) ? count - stream->len : count;
    return count;
}

static uint32_t evq_stream_get_distance(const evq_stream_t *stream, uint32_t head, uint32_t tail)
{
    uint32_t count = stream->len + tail - head;

    count = (count >= stream->len) ? count - stream->len : count;
    return count;
}

evq_status_t evq_stream_create(evq_stream_t **stream, uint32_t size, uint32_t len)
{
    EVQ_ASSERT(NULL != stream, "stream argument is null");

    evq_stream_t *bf = evq_malloc(sizeof(evq_stream_t) + (size * len));
    if (NULL == bf)
    {
        EVQ_LOG_TRACE("Could not allocate memory for evq_stream\n");
        return EVQ_ERROR_NMEM;
    }

    bf->head = 0;
    bf->tail = 0;
    bf->size = size;
    bf->len  = len; // full condition is len - 1
    memset(&bf->data[0], 0, (bf->size * len));

    *stream = bf;
    return EVQ_ERROR_NONE;
}

evq_status_t evq_stream_destroy(evq_stream_t *stream)
{
    EVQ_ASSERT(NULL != stream, "stream argument is null");
    evq_free(stream);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_stream_push(evq_stream_t *stream, const void *ptr)
{
    EVQ_ASSERT(NULL != stream, "stream argument is null");
    EVQ_ASSERT(NULL != ptr, "ptr argument is null");

    evq_status_t st = EVQ_ERROR_NONE;
    uint32_t head   = stream->head;
    uint32_t tail   = stream->tail;
    uint32_t offset = 0;
    uint32_t space  = evq_stream_get_space(stream, head, tail);

    if (space > 0)
    {
        offset = stream->size * head;
        memcpy(&stream->data[offset], ptr, stream->size);
        stream->head = (++head) % (stream->len);
    }
    else
    {
        st = EVQ_ERROR_STREAM_FULL;
    }

    return st;
}

evq_status_t evq_stream_pop(evq_stream_t *stream, void *ptr)
{
    evq_status_t st = EVQ_ERROR_NONE;
    EVQ_ASSERT(NULL != stream, "stream argument is null");
    EVQ_ASSERT(NULL != ptr, "ptr argument is null");
    uint32_t head = stream->head;
    uint32_t tail = stream->tail;
    uint32_t offset = 0;
    uint32_t size = evq_stream_get_distance(stream, tail, head);

    if (size > 0)
    {
        offset = stream->size * tail;
        memcpy(ptr, &stream->data[offset], stream->size);
        stream->tail = (++tail) % (stream->len);
    }
    else
    {
        st = EVQ_ERROR_STREAM_EMPTY;
    }

    return st;
}

evq_status_t evq_stream_size(evq_stream_t *stream, uint32_t *size)
{
    evq_status_t st = EVQ_ERROR_NONE;
    EVQ_ASSERT(NULL != stream, "stream argument is null");
    EVQ_ASSERT(NULL != size, "size argument is null");

    uint32_t head = stream->head;
    uint32_t tail = stream->tail;
    *size         = evq_stream_get_distance(stream, tail, head);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_stream_clear(evq_stream_t *stream)
{
    evq_status_t st = EVQ_ERROR_NONE;
    EVQ_ASSERT(NULL != stream, "stream argument is null");
    // clearing is the same as setting tail to head
    // not actually recommended to use this
    stream->tail = stream->head;
    return st;
}
