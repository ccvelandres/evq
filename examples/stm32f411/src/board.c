#include "board.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>

/** Clock Configuration -> STM32F411 HSE 25MHz
 * SYSCLK     -> 84MHz
 * USB Clock  -> 48MHz
 * AHB Clock  -> 84MHz
 * APB1 Clock -> 42MHz --> low speed domain
 * APB2 Clock -> 84MHz --> high speed domain
 * 
*/

void setupClocks(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
}

void setupGpio(void)
{
    rcc_periph_clock_enable(CFG_LED_RCC_PORT);
    gpio_mode_setup(CFG_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CFG_LED_PIN);
    gpio_set(CFG_LED_PORT, CFG_LED_PIN);
}
