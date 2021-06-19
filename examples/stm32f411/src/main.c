#include <stdlib.h>
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "board.h"
#include "usb.h"

/** Task Configuration */
#define TASK1_STACK_SIZE 1024

static void threadFunction(void *pxArg)
{
    while (1)
    {
        printf("lmao\n");
        gpio_toggle(CFG_LED_PORT, CFG_LED_PIN);
        vTaskDelay(200);
    }
}

static void setupTasks(void)
{
    static TaskHandle_t taskHandle_1;
    xTaskCreate(threadFunction,
                "thread_1",
                TASK1_STACK_SIZE,
                NULL,
                configMAX_PRIORITIES - 1,
                &taskHandle_1);
}

int main(void)
{
    setupClocks();
    setupGpio();
    setupUsb();
    setupTasks();
    setupLog();

    while (!usb_serial_ready())
        ;

    // Start scheduler
    vTaskStartScheduler();
    return 0;
}