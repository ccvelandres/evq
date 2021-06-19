#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "usb.h"

QueueHandle_t queueHandle;

typedef struct
{
    uint32_t len;
    char    *data;
} logMessage;

void logTaskFunction(void *pxArg)
{
    BaseType_t ret;
    while (1)
    {
        logMessage msg;
        ret = xQueueReceive(queueHandle, &msg, portMAX_DELAY);
        if (pdTRUE == ret)
        {
            usb_serial_write(msg.data, msg.len);
            free(msg.data);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void th_log(const char *str, uint32_t len)
{
    BaseType_t ret;
    logMessage msg = {.data = malloc(len), .len = len};
    if (msg.data == NULL) return;

    memcpy(msg.data, str, len);

    ret = xQueueSend(queueHandle, &msg, pdMS_TO_TICKS(5));
    if (pdTRUE != ret) free(msg.data);
}

// evq flush log port
void evq_flush_log(const char *str, uint32_t len) { th_log(str, len); }

void setupLog(void)
{
    static TaskHandle_t logTaskHandle;
    queueHandle = xQueueCreate(64, sizeof(logMessage));
    xTaskCreate(logTaskFunction, "thread_2", 256, NULL, configMAX_PRIORITIES - 1, &logTaskHandle);
}