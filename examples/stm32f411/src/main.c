
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "board.h"

/** Task Configuration */
#define TASK1_STACK_SIZE 1024

void threadFunction(void *pxArg)
{
    while(1)
    {
        gpio_toggle(LED_PORT, LED_PIN);
        vTaskDelay(200);
    }
}

void setupTasks()
{
    BaseType_t ret;
    static TaskHandle_t taskHandle_1;
    xTaskCreate(threadFunction, "thread_1", TASK1_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &taskHandle_1);
}

int main()
{
    setupClocks();
    setupGpio();
    setupTasks();

    // Start scheduler
    vTaskStartScheduler();
    return 0;
}