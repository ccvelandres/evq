#include <string>
#include <mutex>
#include <vector>
#include <queue>
#include <chrono>
#include <condition_variable>

#include <string.h>
#include <evq/evq_log.h>
#include <evq/evq_port.h>

using sys_time_point = std::chrono::system_clock::time_point;

static sys_time_point getTimeoutUntil(const uint32_t &ms)
{
    return std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
}

////////////////////////////////////////////////////////////////////
// Queue wrappers
////////////////////////////////////////////////////////////////////

struct evq_queue_priv_t
{
    uint16_t                    entrySize;
    uint16_t                    queueSize;
    uint16_t                    pushIndex;
    uint16_t                    popIndex;
    std::timed_mutex            mutex;
    std::condition_variable_any cond;
    std::queue<void *>          queue;
};

extern "C" evq_status_t evq_queue_create(evq_queue_t *queue,
                                         uint16_t     entrySize,
                                         uint16_t     queueSize)
{
    EVQ_ASSERT(NULL != queue, "queue is null");

    struct evq_queue_priv_t *priv = new struct evq_queue_priv_t();
    if (NULL == priv) return EVQ_ERROR_NMEM;

    priv->queueSize = queueSize;
    priv->entrySize = entrySize;

    *queue = priv;
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_queue_destroy(evq_queue_t queue)
{
    struct evq_queue_priv_t *priv = static_cast<struct evq_queue_priv_t *>(queue);
    if(NULL != priv)
    {
        EVQ_LOG_TRACE("queue argument is null");
        delete priv;
    }
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_queue_send(evq_queue_t queue,
                                       const void *entry,
                                       uint32_t    timeout)
{
    sys_time_point timeoutMs = getTimeoutUntil(timeout);
    struct evq_queue_priv_t *priv = static_cast<struct evq_queue_priv_t *>(queue);
    EVQ_ASSERT(NULL != priv, "queue is null");

    std::unique_lock<std::timed_mutex> l(priv->mutex, timeoutMs);
    if (!l.owns_lock())
    {
        EVQ_LOG_TRACE("Could not lock resource");
        return EVQ_ERROR_MUTEX;
    }

    if (priv->queueSize <= priv->queue.size())
    {
        EVQ_LOG_TRACE("Queue is full");
        return EVQ_ERROR_QUEUE_FULL;
    }

    uint8_t *data = new uint8_t[priv->entrySize];
    memcpy(data, entry, priv->entrySize);

    priv->queue.push(data);
    priv->cond.notify_all();
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_queue_send_isr(evq_queue_t queue,
                                           const void *entry,
                                           uint32_t    timeout)
{
    return evq_queue_send(queue, entry, timeout);
}

extern "C" evq_status_t evq_queue_receive(evq_queue_t queue,
                                          void       *entry,
                                          uint32_t    timeout)
{
    sys_time_point timeoutMs = getTimeoutUntil(timeout);
    struct evq_queue_priv_t *priv = static_cast<struct evq_queue_priv_t *>(queue);
    EVQ_ASSERT(NULL != priv, "queue is null");

    EVQ_LOG_INFO("test");
    // only when timeout is zero AKA poll
    if ((0 == timeout) && (0 == priv->queue.size()))
    {
        EVQ_LOG_TRACE("Queue is empty");
        return EVQ_ERROR_QUEUE_EMPTY;
    }

    std::unique_lock<std::timed_mutex> l(priv->mutex, timeoutMs);
    if (!l.owns_lock())
    {
        EVQ_LOG_TRACE("Could not lock resource");
        return EVQ_ERROR_MUTEX;
    }

    // Wait until we can get an entry
    while (0 == priv->queue.size())
    {
        auto st = priv->cond.wait_until(l, timeoutMs);
        if (st == std::cv_status::timeout)
        {
            EVQ_LOG_TRACE("wait timeout");
            return EVQ_ERROR_TIMEOUT;
        }
    }

    // pop entry
    uint8_t *data = (uint8_t *)priv->queue.front();
    priv->queue.pop();

    memcpy(entry, data, priv->entrySize);
    delete data;

    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_queue_receive_isr(evq_queue_t queue,
                                              void       *entry,
                                              uint32_t    timeout)
{
    return evq_queue_receive_isr(queue, entry, timeout);
}
