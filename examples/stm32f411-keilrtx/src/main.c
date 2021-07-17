
#include <cmsis_os2.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"

void app_main(void *pxArg)
{
    for(;;){
        gpio_toggle(CFG_LED_PORT, CFG_LED_PIN);
        osDelay(500); // 500ms
    }
}


int main()
{
    osKernelInitialize();
    osThreadNew(app_main, NULL, NULL);
    osKernelStart();

    return 0;
}