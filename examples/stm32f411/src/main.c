#include <stdlib.h>
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <evq/evq_core.h>
#include <evq/evq_log.h>

#include "board.h"
#include "usb.h"

#define CLIENT_STACK_SIZE 1024

static TaskHandle_t evqClientTaskHandle_1;
static StaticTask_t evqClientTaskBuffer_1;
static StackType_t  evqClientStack_1[CLIENT_STACK_SIZE];

static TaskHandle_t evqClientTaskHandle_2;
static StaticTask_t evqClientTaskBuffer_2;
static StackType_t  evqClientStack_2[CLIENT_STACK_SIZE];

static TaskHandle_t evqClientTaskHandle_3;
static StaticTask_t evqClientTaskBuffer_3;
static StackType_t  evqClientStack_3[CLIENT_STACK_SIZE];

static TaskHandle_t evqCoreTaskHandle;
static StaticTask_t evqCoreTaskBuffer;
static StackType_t  evqCoreStack[CLIENT_STACK_SIZE];

const evq_id_t evqClientHandleId_1 = 0xA1;
const evq_id_t evqClientHandleId_2 = 0xA2;
const evq_id_t evqClientHandleId_3 = 0xA3;

static void setupBoard()
{
    setupClocks();
    setupGpio();
    setupUsb();
    setupHeartbeat();
    setupLog();

    while (!usb_serial_ready())
        ;
}

void evqClientTaskFunction_1(void *pxArg)
{
    evq_status_t              st            = EVQ_ERROR_NONE;
    evq_handle_t              handle        = NULL;
    const evq_handle_config_t handle_config = {.handleName   = "client_1",
                                               .handleId     = evqClientHandleId_1,
                                               .queueSize    = 8,
                                               .eventHandler = NULL};

    st = evq_handle_register(&handle, &handle_config);
    configASSERT(st = EVQ_ERROR_NONE);

    while (1)
    {
        static uint32_t msgCount = 0;
        static uint32_t msgSent  = 0;

        evq_message_t msg = NULL;
        evq_message_allocate(&msg, 0);

        st = evq_send(handle, evqClientHandleId_2, msgCount, msg);
        if (EVQ_ERROR_NONE == st)
        {
            msgSent++;
        }
        else
        {
            EVQ_LOG_ERROR("DIRECT: Error(0x%X) sending %d\n", st, msgCount);
        }

        msgCount++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    st = evq_handle_unregister(handle);
}

void evqClientTaskFunction_2(void *pxArg)
{
    evq_status_t              st            = EVQ_ERROR_NONE;
    evq_handle_t              handle        = NULL;
    const evq_handle_config_t handle_config = {.handleName   = "client_2",
                                               .handleId     = evqClientHandleId_2,
                                               .queueSize    = 8,
                                               .eventHandler = NULL};

    st = evq_handle_register(&handle, &handle_config);
    configASSERT(st = EVQ_ERROR_NONE);

    while (1)
    {
        evq_message_t msg = NULL;

        st = evq_receive(handle, &msg, 5);
        if (EVQ_ERROR_NONE == st)
        {
            EVQ_LOG_INFO("evq_message received: %u\n", msg->msgId);

            // destroy message after use
            evq_message_destroy(msg);
        }
    }

    st = evq_handle_unregister(handle);
}

void evqClientTaskFunction_3(void *pxArg)
{
    evq_status_t              st            = EVQ_ERROR_NONE;
    evq_handle_t              handle        = NULL;
    const evq_handle_config_t handle_config = {.handleName   = "client_3",
                                               .handleId     = evqClientHandleId_3,
                                               .queueSize    = 8,
                                               .eventHandler = NULL};

    st = evq_handle_register(&handle, &handle_config);
    configASSERT(st = EVQ_ERROR_NONE);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    st = evq_handle_unregister(handle);
    configASSERT(st = EVQ_ERROR_NONE);
}

void evqCoreTaskFunction(void *pxArg)
{
    for (;;) evq_process();
}

int main(void)
{
    setupBoard();

    // evq_status_t st = evq_init();
    // configASSERT(st == EVQ_ERROR_NONE);

    // evqClientTaskHandle_1 = xTaskCreateStatic(evqClientTaskFunction_1,
    //                                           "evqClientTaskFunction_1",
    //                                           CLIENT_STACK_SIZE,
    //                                           NULL,
    //                                           configMAX_PRIORITIES - 3,
    //                                           evqClientStack_1,
    //                                           &evqClientTaskBuffer_1);
    // evqClientTaskHandle_2 = xTaskCreateStatic(evqClientTaskFunction_2,
    //                                           "evqClientTaskFunction_2",
    //                                           CLIENT_STACK_SIZE,
    //                                           NULL,
    //                                           configMAX_PRIORITIES - 3,
    //                                           evqClientStack_2,
    //                                           &evqClientTaskBuffer_2);
    // evqClientTaskHandle_3 = xTaskCreateStatic(evqClientTaskFunction_3,
    //                                           "evqClientTaskFunction_3",
    //                                           CLIENT_STACK_SIZE,
    //                                           NULL,
    //                                           configMAX_PRIORITIES - 3,
    //                                           evqClientStack_3,
    //                                           &evqClientTaskBuffer_3);

    // evqCoreTaskHandle = xTaskCreateStatic(evqCoreTaskFunction,
    //                                       "evqCoreTaskFunction",
    //                                       CLIENT_STACK_SIZE,
    //                                       NULL,
    //                                       configMAX_PRIORITIES - 3,
    //                                       evqCoreStack,
    //                                       &evqCoreTaskBuffer);

    // Start scheduler
    vTaskStartScheduler();
    return 0;
}