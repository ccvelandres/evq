#include <evq/evq_log.h>
#include <evq/evq_port.h>

#include <FreeRTOS.h>
#include <event_groups.h>

////////////////////////////////////////////////////////////////////
// Egroup wrappers
////////////////////////////////////////////////////////////////////

uint32_t evq_get_time()
{
    // Assuming FreeRTOS tick rate is 1Hz, this should return ms
    return xTaskGetTickCount();
}