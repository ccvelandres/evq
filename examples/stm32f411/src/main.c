
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define LED_RCC_PORT RCC_GPIOC
#define LED_PORT GPIOC
#define LED_PIN GPIO13

int main()
{
    rcc_periph_clock_enable(LED_RCC_PORT);
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);

    gpio_set(LED_PORT, LED_PIN);
    

    while(1)
    {
        for(int i = 0; i < 0xFFFF; ++i) __asm__("nop");
        gpio_toggle(LED_PORT, LED_PIN);
    }

    return 0;
}