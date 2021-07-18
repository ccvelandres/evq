#include <stdbool.h>
#include <stdint.h>

#define __cyg_buffer_size 128
#define __rtos_freertos   1

/** The way we make atomic writes is just disabling global interrupts */
uint32_t inline __lock_buffer()
{
    register uint32_t primask;
    __asm__ volatile("MRS %0, PRIMASK" : "=r"(primask));
    primask = !primask;
    if (primask) __asm__("cpsid i");
    return primask;
}

void inline __unlock_buffer(bool primask)
{
    if (primask) __asm__("cpsie i");
}

#if defined(__rtos_freertos)
#include <FreeRTOS.h>
extern void               *pxCurrentTCB;
static inline unsigned int __cyg_get_thread_id(void) { return (unsigned int)pxCurrentTCB; }
#else
static inline unsigned int __cyg_get_thread_id(void) { return 0; }
#endif
#define US_PER_TICK 10
static volatile uint32_t   __cyg_timestamp;
static inline unsigned int __cyg_get_timestamp(void) { return __cyg_timestamp; }

static inline unsigned int __cyg_get_active_isr(void)
{
    // test get active vector id from SCB->ICSR
    const uint32_t *__scb                     = (uint32_t *)0xE000ED04;
    const uint32_t  __scb_icsr_vectactive_msk = 0xFF;
    return ((*__scb) & __scb_icsr_vectactive_msk);
}

typedef struct __profile_entry
{
    unsigned int is_enter;
    unsigned int thread_id;
    unsigned int timestamp;
    void        *this_fn;
    void        *call_site;
} __cyg_profile_entry_t;

static bool                  __cyg_profile_stopped     = true;
static bool                  __cyg_profile_enabled     = false;
static unsigned int          __cyg_profile_flush_head  = 0;
static unsigned int          __cyg_profile_buffer_head = 0;
static __cyg_profile_entry_t __cyg_profile_buffer[__cyg_buffer_size]
    = {[0 ...(__cyg_buffer_size - 1)] = {}};

static bool __cyg_flush_uart(void *ptr, uint16_t len);

__attribute__((no_instrument_function)) static void inline __cyg_flush()
{
    uint32_t size = __cyg_buffer_size + __cyg_profile_buffer_head - __cyg_profile_flush_head;
    size          = (size >= __cyg_buffer_size) ? size - __cyg_buffer_size : size;

    // do flush on size/4
    if (size >= (__cyg_buffer_size / 8))
    {
        if (__cyg_profile_flush_head > __cyg_profile_buffer_head)
        {
            size = __cyg_buffer_size - __cyg_profile_flush_head;
        }

        bool flushed = __cyg_flush_uart((void *)&__cyg_profile_buffer[__cyg_profile_flush_head],
                                        sizeof(__cyg_profile_entry_t) * size);
        if (flushed)
            __cyg_profile_flush_head = (__cyg_profile_flush_head + size) % __cyg_buffer_size;
    }
}

__attribute__((no_instrument_function)) static void __cyg_store_entry(unsigned int is_enter,
                                                                      void        *this_fn,
                                                                      void        *call_site)
{
    uint32_t is_locked                                        = __lock_buffer();
    unsigned int active_isr = __cyg_get_active_isr();
    __cyg_profile_buffer[__cyg_profile_buffer_head].thread_id = active_isr ? active_isr :__cyg_get_thread_id();
    __cyg_profile_buffer[__cyg_profile_buffer_head].timestamp = __cyg_get_timestamp();
    __cyg_profile_buffer[__cyg_profile_buffer_head].this_fn   = this_fn;
    __cyg_profile_buffer[__cyg_profile_buffer_head].call_site = call_site;
    __cyg_profile_buffer[__cyg_profile_buffer_head].is_enter  = (is_enter << 0) | ((!!active_isr) << 1);
    __cyg_profile_buffer_head = (__cyg_profile_buffer_head + 1) % __cyg_buffer_size;
    __cyg_flush();
    __unlock_buffer(is_locked);
}

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (__cyg_profile_enabled)
    {
        __cyg_store_entry(1, this_fn, call_site);
    }
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (__cyg_profile_enabled)
    {
        __cyg_store_entry(0, this_fn, call_site);
    }
}

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>

void cyg_profiler_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_DMA2);
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

    // using PB6 - tx, PB7 - rx
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);

    usart_set_baudrate(USART1, 230400);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_enable(USART1);

    dma_stream_reset(DMA2, DMA_STREAM7);
    dma_set_peripheral_address(DMA2, DMA_STREAM7, (uint32_t)&USART1_DR);
    dma_set_priority(DMA2, DMA_STREAM7, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA2, DMA_STREAM7, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM7, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM7);
    dma_set_transfer_mode(DMA2, DMA_STREAM7, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_channel_select(DMA2, DMA_STREAM7, DMA_SxCR_CHSEL_4);

    // usart1_tx -> dma2_channel7
    nvic_set_priority(NVIC_DMA2_STREAM7_IRQ, 0);
    nvic_enable_irq(NVIC_DMA2_STREAM7_IRQ);
}

void cyg_profiler_start(void)
{
    static uint8_t start_id[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    __cyg_flush_uart(start_id, sizeof(start_id));
    __cyg_profile_enabled = true;
    __cyg_profile_stopped = false;
}

void cyg_profiler_end(void)
{
    static uint8_t end_id[] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08};
    __cyg_profile_enabled   = false;
    if(!__cyg_profile_stopped) __cyg_profile_stopped = __cyg_flush_uart(end_id, sizeof(end_id));
}

__attribute__((no_instrument_function)) bool __cyg_flush_uart(void *ptr, uint16_t len)
{
    // return if busy
    if (DMA2_S7CR & DMA_SxCR_EN) return false;

    dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t)ptr);
    dma_set_number_of_data(DMA2, DMA_STREAM7, len);

    dma_enable_stream(DMA2, DMA_STREAM7);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM7);
    usart_enable_tx_dma(USART1);
    return true;
}

void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF))
    {
        timer_clear_flag(TIM2, TIM_SR_UIF);
        __cyg_timestamp++;
    }
}

void dma2_stream7_isr(void)
{
    if (dma_get_interrupt_flag(DMA2, DMA_STREAM7, DMA_TCIF))
    {
        dma_clear_interrupt_flags(DMA2, DMA_STREAM7, DMA_TCIF);
        usart_disable_tx_dma(USART1);
        dma_disable_transfer_complete_interrupt(DMA2, DMA_STREAM7);
        dma_disable_stream(DMA2, DMA_STREAM7);
    }
}