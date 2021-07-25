#include <stdbool.h>
#include <stdint.h>

#include "profiler.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

/** The way we make atomic writes is just disabling global interrupts */
__attribute__((no_instrument_function)) uint32_t inline __lock_profiler()
{
    register uint32_t primask;
    __asm__ volatile("MRS %0, PRIMASK" : "=r"(primask));
    primask = !primask;
    if (primask) __asm__("cpsid i");
    return primask;
}

__attribute__((no_instrument_function)) void inline __unlock_profiler(bool primask)
{
    if (primask) __asm__("cpsie i");
}

static bool __cyg_profile_stopped = true;
static bool __cyg_profile_enabled = false;

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (__cyg_profile_enabled)
    {
        uint32_t is_locked = __lock_profiler();
        __cyg_profiler_store(1, this_fn, call_site);
        __unlock_profiler(is_locked);
    }
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (__cyg_profile_enabled)
    {
        uint32_t is_locked = __lock_profiler();
        __cyg_profiler_store(0, this_fn, call_site);
        __unlock_profiler(is_locked);
    }
}

__attribute__((no_instrument_function)) void __cyg_profiler_start(void)
{
    static uint8_t start_id[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    __cyg_profiler_flush(start_id, sizeof(start_id));
    __cyg_profile_enabled = true;
    __cyg_profile_stopped = false;
}

__attribute__((no_instrument_function)) void __cyg_profiler_end(void)
{
    static uint8_t end_id[] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08};
    __cyg_profile_enabled   = false;
    if (!__cyg_profile_stopped)
        __cyg_profile_stopped = __cyg_profiler_flush(end_id, sizeof(end_id));
}

__attribute__((no_instrument_function)) void __cyg_profiler_init()
{
    // Init timer for profiler time base
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_reset_pulse(RST_TIM2);

    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(TIM2, ((rcc_apb1_frequency * 2) / 1000000) - 1); // get 1Mhz from apb1
    timer_set_period(TIM2, US_PER_TICK - 1);                             // tick per 1ms
    timer_disable_preload(TIM2);
    timer_continuous_mode(TIM2);
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    timer_enable_counter(TIM2);
    nvic_enable_irq(NVIC_TIM2_IRQ);

    // init flush method
    __cyg_profiler_init_flush();
}

__attribute__((no_instrument_function)) void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF))
    {
        timer_clear_flag(TIM2, TIM_SR_UIF);
        __cyg_timestamp++;
    }
}
