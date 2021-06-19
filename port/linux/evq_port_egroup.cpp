#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>

#include <evq/evq_log.h>
#include <evq/evq_port.h>

using sys_time_point = std::chrono::system_clock::time_point;

static sys_time_point getTimeNow() { return std::chrono::system_clock::now(); }

static sys_time_point getTimeoutUntil(const uint32_t &ms)
{
    return getTimeNow() + std::chrono::milliseconds(ms);
}

////////////////////////////////////////////////////////////////////
// Egroup wrappers
////////////////////////////////////////////////////////////////////

struct evq_egroup_priv_t
{
    // condition variable doesn't seem to work well
    // with timed_mutex... or I'm an idiot
    std::mutex mutex;
    uint32_t   flags;
};

static std::unique_ptr<evq_egroup_priv_t> g_egroups[16];

extern "C" evq_status_t evq_egroup_create(evq_egroup_t *egroup)
{
    struct evq_egroup_priv_t *priv = new struct evq_egroup_priv_t();
    if (NULL == priv) return EVQ_ERROR_NMEM;
    *egroup = priv;
    EVQ_LOG_TRACE("egroup created @ %p\n", priv);
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_egroup_destroy(evq_egroup_t egroup)
{
    struct evq_egroup_priv_t *priv = static_cast<struct evq_egroup_priv_t *>(egroup);
    EVQ_ASSERT(NULL != priv, "egroup is null");
    EVQ_LOG_TRACE("egroup destroy @ %p\n", priv);
    delete priv;
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_egroup_set(evq_egroup_t egroup, uint32_t flags, uint32_t timeout)
{
    sys_time_point            timeoutMs = getTimeoutUntil(timeout);
    struct evq_egroup_priv_t *priv      = static_cast<struct evq_egroup_priv_t *>(egroup);
    EVQ_ASSERT(NULL != priv, "egroup is null");

    {
        std::unique_lock<std::mutex> l(priv->mutex);
        priv->flags |= flags;
    }
    return EVQ_ERROR_NONE;
}

extern "C" evq_status_t evq_egroup_set_isr(evq_egroup_t egroup, uint32_t flags, uint32_t timeout)
{
    return evq_egroup_set(egroup, flags, timeout);
}

extern "C" evq_status_t evq_egroup_wait(evq_egroup_t egroup,
                                        uint32_t     flags,
                                        uint32_t    *matchFlag,
                                        bool         waitForAll,
                                        uint32_t     timeout)
{
    evq_status_t              st        = EVQ_ERROR_NONE;
    sys_time_point            timeoutMs = getTimeoutUntil(timeout);
    uint32_t                  _flag     = 0;
    struct evq_egroup_priv_t *priv      = static_cast<struct evq_egroup_priv_t *>(egroup);
    EVQ_ASSERT(NULL != priv, "egroup is null");

    std::unique_lock<std::mutex> l(priv->mutex);

    do
    {
        _flag = priv->flags & flags;
        if (waitForAll ? (_flag == flags) : _flag)
        {
            // we got event
            *matchFlag = (priv->flags & flags);
            priv->flags &= ~(*matchFlag);
            return EVQ_ERROR_NONE;
        }
        l.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        l.lock();
    } while (std::chrono::system_clock::now() < timeoutMs);
    return EVQ_ERROR_TIMEOUT;
}
