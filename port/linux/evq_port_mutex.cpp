#include <chrono>
#include <mutex>

#include <evq/evq_log.h>
#include <evq/evq_port.h>

using sys_time_point = std::chrono::system_clock::time_point;

static sys_time_point getTimeoutUntil(const uint32_t &ms)
{
    return std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
}

////////////////////////////////////////////////////////////////////
// Mutex wrappers
////////////////////////////////////////////////////////////////////

struct evq_mutex_priv_t
{
    std::timed_mutex mutex;
};

extern "C" evq_status_t evq_mutex_create(evq_mutex_t *mutex)
{
    struct evq_mutex_priv_t *priv = new struct evq_mutex_priv_t();
    if (NULL == priv) return EVQ_ERROR_NMEM;
    *mutex = priv;
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_mutex_destroy(evq_mutex_t mutex)
{
    struct evq_mutex_priv_t *priv = static_cast<struct evq_mutex_priv_t *>(mutex);
    EVQ_ASSERT(priv != NULL, "mutex is null");

    delete priv;
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_mutex_lock(evq_mutex_t mutex, uint32_t timeout)
{
    sys_time_point timeoutMs = getTimeoutUntil(timeout);
    struct evq_mutex_priv_t *priv = static_cast<struct evq_mutex_priv_t *>(mutex);
    EVQ_ASSERT(priv != NULL, "mutex is null");

    if (EVQ_TIMEOUT_MAX == timeout)
        priv->mutex.lock();
    else if (!priv->mutex.try_lock_until(timeoutMs))
        return EVQ_ERROR_MUTEX;

    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_mutex_lock_isr(evq_mutex_t mutex, uint32_t timeout)
{
    return evq_mutex_lock(mutex, timeout);
}

extern "C" evq_status_t evq_mutex_unlock(evq_mutex_t mutex)
{
    struct evq_mutex_priv_t *priv = static_cast<struct evq_mutex_priv_t *>(mutex);
    EVQ_ASSERT(priv != NULL, "mutex is null");

    priv->mutex.unlock();
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_mutex_unlock_isr(evq_mutex_t mutex)
{
    return evq_mutex_unlock(mutex);
}
