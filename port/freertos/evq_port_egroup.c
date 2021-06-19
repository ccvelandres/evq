#include <evq/evq_log.h>
#include <evq/evq_port.h>

#include <FreeRTOS.h>
#include <event_groups.h>

////////////////////////////////////////////////////////////////////
// Egroup wrappers
////////////////////////////////////////////////////////////////////

evq_status_t evq_egroup_create(evq_egroup_t *egroup)
{
    EventGroupHandle_t handle = xEventGroupCreate();
    if (NULL == handle)
    {
        return EVQ_ERROR_NMEM;
    }

    *egroup = handle;
    EVQ_LOG_TRACE("egroup created @ %p\n", handle);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_destroy(evq_egroup_t egroup)
{
    EventGroupHandle_t handle = (EventGroupHandle_t)egroup;
    EVQ_ASSERT(NULL != egroup, "egroup is null");

    vEventGroupDelete(handle);
    EVQ_LOG_TRACE("egroup destroy @ %p\n", handle);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_set(evq_egroup_t egroup, uint32_t flags, uint32_t timeout)
{
    EventGroupHandle_t handle = (EventGroupHandle_t)egroup;
    EVQ_ASSERT(NULL != egroup, "egroup is null");

    (void)xEventGroupSetBits(handle, flags);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_set_isr(evq_egroup_t egroup, uint32_t flags, uint32_t timeout)
{
    BaseType_t         taskYield, ret;
    EventGroupHandle_t handle = (EventGroupHandle_t)egroup;
    EVQ_ASSERT(NULL != egroup, "egroup is null");

    ret = xEventGroupSetBitsFromISR(handle, flags, &taskYield);
    if (pdFAIL != ret) portYIELD_FROM_ISR(taskYield);
    return EVQ_ERROR_NONE;
}

evq_status_t evq_egroup_wait(evq_egroup_t egroup,
                                        uint32_t     flags,
                                        uint32_t    *matchFlag,
                                        bool         waitForAll,
                                        uint32_t     timeout)
{
    EventBits_t        bits;
    EventGroupHandle_t handle = (EventGroupHandle_t)egroup;
    EVQ_ASSERT(NULL != egroup, "egroup is null");
    EVQ_ASSERT(NULL != matchFlag, "matchFlag is null");

    bits = xEventGroupWaitBits(handle,
                               flags,
                               pdTRUE,
                               waitForAll ? pdTRUE : pdFALSE,
                               pdMS_TO_TICKS(timeout));

    *matchFlag = bits;
    return EVQ_ERROR_NONE;
}
