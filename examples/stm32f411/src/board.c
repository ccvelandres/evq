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
#define CFG_RCC_PLLM (25)
#define CFG_RCC_PLLN (336)
#define CFG_RCC_PLLP (4)
#define CFG_RCC_PLLQ (7)
#define CFG_RCC_PLLR (0)

void setupClocks()
{
    // Use HSI for configuration
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);

    rcc_osc_on(RCC_HSE);
    rcc_wait_for_osc_ready(RCC_HSE);

    rcc_set_hpre(RCC_CFGR_HPRE_DIV_NONE);
    rcc_set_ppre1(RCC_CFGR_PPRE_DIV_2);
    rcc_set_ppre2(RCC_CFGR_PPRE_DIV_NONE);

    rcc_osc_off(RCC_PLL);
    rcc_set_main_pll_hse(CFG_RCC_PLLM, CFG_RCC_PLLN, CFG_RCC_PLLP, CFG_RCC_PLLQ, CFG_RCC_PLLR);
    rcc_osc_on(RCC_PLL);
    rcc_wait_for_osc_ready(RCC_PLL);

    flash_icache_enable();
    flash_dcache_enable();
    flash_set_ws(FLASH_ACR_LATENCY_2WS | FLASH_ACR_DCEN | FLASH_ACR_ICEN);

    rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
    rcc_wait_for_sysclk_status(RCC_PLL);

    // update frequency variables from libopencm3
    rcc_ahb_frequency = 84000000;
    rcc_apb1_frequency = 42000000;
    rcc_apb2_frequency = 84000000;

    rcc_osc_off(RCC_HSI);
    rcc_osc_off(RCC_LSI);
}

void setupGpio()
{
    rcc_periph_clock_enable(LED_RCC_PORT);
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_set(LED_PORT, LED_PIN);
}
