
#include <evq/evq_log.h>
#include <evq/evq_port.h>

#include <FreeRTOS.h>
#include <semphr.h>

////////////////////////////////////////////////////////////////////
// Mutex wrappers
////////////////////////////////////////////////////////////////////

evq_status_t evq_mutex_create(evq_mutex_t *mutex)
{
    SemaphoreHandle_t handle = xSemaphoreCreateMutex();
    if (NULL == handle)
    {
        return EVQ_ERROR_NMEM;
    }

    *mutex = handle;
    EVQ_LOG_TRACE("mutex created @ %p\n", handle);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_mutex_destroy(evq_mutex_t mutex)
{
    SemaphoreHandle_t handle = (SemaphoreHandle_t)mutex;
    EVQ_ASSERT(NULL != mutex, "mutex is null");

    vSemaphoreDelete(handle);
    EVQ_LOG_TRACE("mutex destroy @ %p\n", handle);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_mutex_lock(evq_mutex_t mutex, uint32_t timeout)
{
    BaseType_t        ret;
    SemaphoreHandle_t handle = (SemaphoreHandle_t)mutex;
    EVQ_ASSERT(NULL != mutex, "mutex is null");

    ret = xSemaphoreTake(handle, pdMS_TO_TICKS(timeout));

    return ret == pdTRUE ? EVQ_ERROR_NONE : EVQ_ERROR_MUTEX;
}

evq_status_t evq_mutex_lock_isr(evq_mutex_t mutex, uint32_t timeout)
{
    (void)timeout;
    BaseType_t ret, taskYield;

    SemaphoreHandle_t handle = (SemaphoreHandle_t)mutex;
    EVQ_ASSERT(NULL != mutex, "mutex is null");

    ret = xSemaphoreTakeFromISR(handle, &taskYield);
    portYIELD_FROM_ISR(taskYield);

    return ret == pdTRUE ? EVQ_ERROR_NONE : EVQ_ERROR_MUTEX;
}

evq_status_t evq_mutex_unlock(evq_mutex_t mutex)
{
    BaseType_t        ret, taskYield;
    SemaphoreHandle_t handle = (SemaphoreHandle_t)mutex;
    EVQ_ASSERT(NULL != mutex, "mutex is null");

    ret = xSemaphoreGive(handle);

    return ret == pdTRUE ? EVQ_ERROR_NONE : EVQ_ERROR_MUTEX;
}

evq_status_t evq_mutex_unlock_isr(evq_mutex_t mutex)
{
    BaseType_t        ret, taskYield;
    SemaphoreHandle_t handle = (SemaphoreHandle_t)mutex;
    EVQ_ASSERT(NULL != mutex, "mutex is null");

    ret = xSemaphoreGiveFromISR(handle, &taskYield);
    portYIELD_FROM_ISR(taskYield);

    return ret == pdTRUE ? EVQ_ERROR_NONE : EVQ_ERROR_MUTEX;
}
