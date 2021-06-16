#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include <evq/evq_log.h>
#include <evq/evq_port.h>

#define MSEC_IN_SEC  1000
#define NSEC_IN_MSEC 1000000

////////////////////////////////////////////////////////////////////
// Memory allocation function wrappers
////////////////////////////////////////////////////////////////////

void *evq_malloc(uint32_t size) { return malloc(size); }

void *evq_calloc(uint32_t nmemb, uint32_t size) { return calloc(nmemb, size); }

void *evq_realloc(void *ptr, uint32_t size) { return realloc(ptr, size); }

void evq_free(void *ptr) { free(ptr); }

////////////////////////////////////////////////////////////////////
// Syncronization wrappers
////////////////////////////////////////////////////////////////////

typedef struct
{
    pthread_mutex_t mutex;
} evq_mutex_priv_t;

evq_status_t evq_mutex_create(evq_mutex_t *mutex)
{
    int                 st  = 0;
    evq_status_t        ret = EVQ_ERROR_MUTEX;
    pthread_mutexattr_t attr;
    evq_mutex_priv_t   *priv = NULL;

    st = pthread_mutexattr_init(&attr);
    if (0 != st) goto exit;

    priv = evq_malloc(sizeof(evq_mutex_priv_t));
    if (NULL == priv) goto cleanup_1;

    st = pthread_mutex_init(&priv->mutex, &attr);
    if (0 != st) goto fail;

    *mutex = priv;
    ret    = EVQ_ERROR_NONE;
    goto exit;
fail:
    evq_free(priv);
cleanup_1:
    pthread_mutexattr_destroy(&attr);
exit:
    return ret;
}

evq_status_t evq_mutex_destroy(evq_mutex_t mutex)
{
    int               st   = 0;
    evq_mutex_priv_t *priv = (evq_mutex_priv_t *)mutex;
    EVQ_ASSERT(priv != NULL, "Null argument");

    st = pthread_mutex_destroy(&priv->mutex);
    if (0 != st)
    {
        EVQ_LOG_TRACE("Error during mutex destroy: %d\n", st);
        return EVQ_ERROR_MUTEX;
    }

    return EVQ_ERROR_NONE;
}

evq_status_t evq_mutex_lock(evq_mutex_t mutex, uint32_t timeout)
{
    int               st   = 0;
    evq_mutex_priv_t *priv = (evq_mutex_priv_t *)mutex;
    EVQ_ASSERT(priv != NULL, "Null argument");

    if (EVQ_TIMEOUT_MAX == timeout)
    {
        st = pthread_mutex_lock(&priv->mutex);
    }
    else
    {
        struct timespec ts;
        memset(&ts, 0, sizeof(struct timespec));
        ts.tv_sec  = timeout / MSEC_IN_SEC;
        ts.tv_nsec = (timeout % MSEC_IN_SEC) * NSEC_IN_MSEC;

        st = pthread_mutex_timedlock(&priv->mutex, &ts);
    }

    if (0 != st)
    {
        EVQ_LOG_TRACE("Error during mutex lock: %d\n", st);
        return EVQ_ERROR_MUTEX;
    }

    return EVQ_ERROR_NONE;
}

evq_status_t evq_mutex_lock_isr(evq_mutex_t mutex, uint32_t timeout)
{
    // just redirect since there's no isr in linux
    return evq_mutex_lock(mutex, timeout);
}

evq_status_t evq_mutex_unlock(evq_mutex_t mutex)
{
    int               st   = 0;
    evq_mutex_priv_t *priv = (evq_mutex_priv_t *)mutex;
    EVQ_ASSERT(priv != NULL, "Null argument");

    st = pthread_mutex_unlock(&priv->mutex);
    if (0 != st)
    {
        EVQ_LOG_TRACE("Error during mutex unlock: %d\n", st);
        return EVQ_ERROR_MUTEX;
    }

    return EVQ_ERROR_NONE;
}

evq_status_t evq_mutex_unlock_isr(evq_mutex_t mutex)
{
    // just redirect since there's no isr in linux
    return evq_mutex_unlock(mutex);
}

typedef struct
{
    // might need mutex here
    uint32_t        flags;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} evq_egroup_priv_t;

evq_status_t evq_egroup_create(evq_egroup_t *egroup)
{
    int                 st  = 0;
    evq_status_t        ret = EVQ_ERROR_EGROUP;
    pthread_condattr_t  condattr;
    pthread_mutexattr_t mutexattr;
    evq_egroup_priv_t  *priv = NULL;

    st = pthread_condattr_init(&condattr) || pthread_mutexattr_init(&mutexattr);
    if (0 != st) goto exit;

    priv = evq_malloc(sizeof(evq_egroup_priv_t));
    if (NULL == priv) goto exit;

    st = pthread_cond_init(&priv->cond, &condattr)
      || pthread_mutex_init(&priv->mutex, &mutexattr);
    if (0 != st) goto cleanup_1;

    *egroup = priv;
    ret     = EVQ_ERROR_NONE;
    goto exit;
cleanup_1:
    evq_free(priv);
exit:
    pthread_mutexattr_destroy(&mutexattr);
    pthread_condattr_destroy(&condattr);
    return ret;
}

evq_status_t evq_egroup_destroy(evq_egroup_t egroup)
{
    int                st   = 0;
    evq_egroup_priv_t *priv = (evq_egroup_priv_t *)egroup;
    EVQ_ASSERT(NULL != priv, "Null argument not allowed");

    st = pthread_cond_destroy(&priv->cond);
    if (0 != st)
    {
        EVQ_LOG_TRACE("Error during egroup destroy: %d\n", st);
        return EVQ_ERROR_EGROUP;
    }

    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_set(evq_egroup_t egroup, uint32_t flags)
{
    int                st   = 0;
    evq_egroup_priv_t *priv = (evq_egroup_priv_t *)egroup;
    EVQ_ASSERT(NULL != priv, "Null argument not allowed");

    pthread_mutex_lock(&priv->mutex);
    priv->flags |= flags;
    pthread_mutex_unlock(&priv->mutex);
    pthread_cond_broadcast(&priv->cond);

    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_set_isr(evq_egroup_t egroup, uint32_t flags)
{
    return evq_egroup_set(egroup, flags);
}

evq_status_t evq_egroup_wait(evq_egroup_t egroup,
                             uint32_t     flags,
                             uint32_t    *matchFlag,
                             bool         waitForAll,
                             uint32_t     timeout)
{
    int                st   = 0;
    evq_egroup_priv_t *priv = (evq_egroup_priv_t *)egroup;
    EVQ_ASSERT(NULL != priv, "Null argument not allowed");

    pthread_mutex_lock(&priv->mutex);

    // loop until flags match
    while (waitForAll ? !((priv->flags & flags) == flags)
                      : !(priv->flags & flags))
    {
        if (timeout == EVQ_TIMEOUT_MAX)
        {
            st = pthread_cond_wait(&priv->cond, &priv->mutex);
        }
        else
        {
            struct timespec ts;
            memset(&ts, 0, sizeof(struct timespec));
            ts.tv_sec  = timeout / MSEC_IN_SEC;
            ts.tv_nsec = (timeout % MSEC_IN_SEC) * NSEC_IN_MSEC;
            st         = pthread_cond_timedwait(&priv->cond, &priv->mutex, &ts);
        }
    }

    // clear flags
    *matchFlag = (priv->flags & flags);
    priv->flags &= ~(*matchFlag);
    pthread_mutex_unlock(&priv->mutex);

    if (0 != st)
    {
        EVQ_LOG_TRACE("Error during egroup wait: %d\n", st);
        return EVQ_ERROR_TIMEOUT;
    }

    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_wait_isr(evq_egroup_t egroup,
                                 uint32_t     flags,
                                 uint32_t    *matchFlag,
                                 bool         waitForAll,
                                 uint32_t     timeout)
{
    return evq_egroup_wait(egroup, flags, matchFlag, waitForAll, timeout);
}

////////////////////////////////////////////////////////////////////
// Logging wrappers
////////////////////////////////////////////////////////////////////

uint32_t evq_flush_log(const char *str, uint32_t len)
{
    fwrite(str, sizeof(char), len, stdout);
}