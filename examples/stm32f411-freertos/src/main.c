#include <stdlib.h>
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <evq/evq_core.h>
#include <evq/evq_log.h>

#include "board.h"
#include "usb.h"
#include "profiler.h"

#define EVQ_TASK_STACK_SIZE 1024

const evq_id_t evqClientReceiverId = 0xB0;

static TaskHandle_t evqReceiverTaskHandle;
static StaticTask_t evqReceiverTaskBuffer;
static StackType_t  evqReceiverStack[EVQ_TASK_STACK_SIZE];

static TaskHandle_t evqCoreTaskHandle;
static StaticTask_t evqCoreTaskBuffer;
static StackType_t  evqCoreStack[EVQ_TASK_STACK_SIZE];

typedef struct
{
    evq_id_t    id;
    const char *name;
    uint32_t    intervalMs;
} evqClientConfig_t;

static const evqClientConfig_t evqClientConfig[] = {
    {.id = 0x00, .name = "client_1", .intervalMs = 250},
    {.id = 0x01, .name = "client_2", .intervalMs = 260},
    {.id = 0x02, .name = "client_3", .intervalMs = 270},
    {.id = 0x03, .name = "client_4", .intervalMs = 280},
    {.id = 0x04, .name = "client_5", .intervalMs = 290},
};
#define evqClientCount (sizeof(evqClientConfig) / sizeof(evqClientConfig_t))

static TaskHandle_t evqClientTaskHandle[evqClientCount];
static StaticTask_t evqClientTaskBuffer[evqClientCount];
static StackType_t  evqClientStack[evqClientCount][EVQ_TASK_STACK_SIZE];

void evqReceiverTaskFunction(void *pxArg)
{
    evq_status_t              st            = EVQ_ERROR_NONE;
    evq_handle_t              handle        = NULL;
    const evq_handle_config_t handle_config = {.handleName   = "receiver",
                                               .handleId     = evqClientReceiverId,
                                               .streamSize   = 8,
                                               .eventHandler = NULL};

    uint32_t lastMsgId[evqClientCount] = {};

    st = evq_handle_register(&handle, &handle_config);
    configASSERT(st == EVQ_ERROR_NONE);

    while (1)
    {
        evq_message_t msg = NULL;

        st = evq_receive(handle, &msg, EVQ_TIMEOUT_MAX);
        if (EVQ_ERROR_NONE == st)
        {
            uint32_t index = msg->srcId;
            // Verify that msgId send is lastMsgId + 1
            if (lastMsgId[index] != msg->msgId)
            {
                EVQ_LOG_ERROR("Dropped message Id from %x, expected %u, actual %u\n",
                              msg->srcId,
                              msg->msgId,
                              lastMsgId[index]);
            }
            lastMsgId[index] = msg->msgId + 1;

            EVQ_LOG_INFO("Received messages: %08u, %08u, %08u, %08u, %08u\n",
                         lastMsgId[0],
                         lastMsgId[1],
                         lastMsgId[2],
                         lastMsgId[3],
                         lastMsgId[4]);
            // destroy message after use
            evq_message_destroy(msg);
        }
    }

    st = evq_handle_unregister(handle);
    configASSERT(st == EVQ_ERROR_NONE);
}

void evqClientTaskFunction(void *pxArg)
{
    uint32_t                  clientIndex   = (uint32_t)pxArg;
    evq_status_t              st            = EVQ_ERROR_NONE;
    evq_handle_t              handle        = NULL;
    const evq_handle_config_t handle_config = {.handleName   = evqClientConfig[clientIndex].name,
                                               .handleId     = evqClientConfig[clientIndex].id,
                                               .streamSize   = 8,
                                               .eventHandler = NULL};

    // Start from msg 1 since receiver expects 0->1
    uint32_t currentMsgCount = 1;

    st = evq_handle_register(&handle, &handle_config);
    configASSERT(st == EVQ_ERROR_NONE);

    while (1)
    {
        evq_message_t msg = NULL;

        st = evq_message_allocate(&msg, 0);
        configASSERT(msg != NULL);

        st = evq_send(handle, evqClientReceiverId, currentMsgCount, msg);
        if (EVQ_ERROR_NONE != st)
        {
            EVQ_LOG_ERROR("DIRECT: Error(0x%X) sending %d\n", st, currentMsgCount);
        }

        currentMsgCount++;
        vTaskDelay(pdMS_TO_TICKS(evqClientConfig[clientIndex].intervalMs));
    }
    st = evq_handle_unregister(handle);
    configASSERT(st == EVQ_ERROR_NONE);
}

void evqCoreTaskFunction(void *pxArg)
{
    for (;;) {
        evq_process();
        if(xTaskGetTickCount() > pdMS_TO_TICKS(5000))
        {
            __cyg_profiler_end();
        }
    }
}

int main(void)
{
    setupBoard();

    __cyg_profiler_init();
    __cyg_profiler_start();
    evq_status_t st = evq_init();
    configASSERT(st == EVQ_ERROR_NONE);

    evqReceiverTaskHandle = xTaskCreateStatic(evqReceiverTaskFunction,
                                              "evqReceiverTaskFunction",
                                              EVQ_TASK_STACK_SIZE,
                                              NULL,
                                              configMAX_PRIORITIES - 3,
                                              evqReceiverStack,
                                              &evqReceiverTaskBuffer);

    for (uint32_t i = 0; i < evqClientCount; ++i)
    {
        evqClientTaskHandle[i] = xTaskCreateStatic(evqClientTaskFunction,
                                                   evqClientConfig[i].name,
                                                   EVQ_TASK_STACK_SIZE,
                                                   (void *)i,
                                                   configMAX_PRIORITIES - 3,
                                                   evqClientStack[i],
                                                   &evqClientTaskBuffer[i]);
    }

    evqCoreTaskHandle = xTaskCreateStatic(evqCoreTaskFunction,
                                          "evqCoreTaskFunction",
                                          EVQ_TASK_STACK_SIZE,
                                          NULL,
                                          configMAX_PRIORITIES - 3,
                                          evqCoreStack,
                                          &evqCoreTaskBuffer);

    // Start scheduler
    vTaskStartScheduler();
    return 0;
}