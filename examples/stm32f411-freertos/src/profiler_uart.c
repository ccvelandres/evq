#include "profiler.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#define __cyg_buffer_size 128

static unsigned int          __cyg_profile_flush_head  = 0;
static unsigned int          __cyg_profile_buffer_head = 0;
static __cyg_profile_entry_t __cyg_profile_buffer[__cyg_buffer_size]
    = {[0 ...(__cyg_buffer_size - 1)] = {}};

__attribute__((no_instrument_function)) void __cyg_profiler_init_uart(void)
{
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_DMA2);

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

__attribute__((no_instrument_function)) bool __cyg_profiler_flush_uart(void *ptr, uint16_t len)
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

__attribute__((no_instrument_function)) void dma2_stream7_isr(void)
{
    if (dma_get_interrupt_flag(DMA2, DMA_STREAM7, DMA_TCIF))
    {
        dma_clear_interrupt_flags(DMA2, DMA_STREAM7, DMA_TCIF);
        usart_disable_tx_dma(USART1);
        dma_disable_transfer_complete_interrupt(DMA2, DMA_STREAM7);
        dma_disable_stream(DMA2, DMA_STREAM7);
    }
}

__attribute__((no_instrument_function)) static void inline cyg_flush_block_uart()
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

        bool flushed = __cyg_profiler_flush((void *)&__cyg_profile_buffer[__cyg_profile_flush_head],
                                            sizeof(__cyg_profile_entry_t) * size);
        if (flushed)
            __cyg_profile_flush_head = (__cyg_profile_flush_head + size) % __cyg_buffer_size;
    }
}

__attribute__((no_instrument_function)) void __cyg_profiler_store_uart(unsigned int is_enter,
                                                                       void        *this_fn,
                                                                       void        *call_site)
{
    unsigned int active_isr = ____cyg_get_active_isr();
    __cyg_profile_buffer[__cyg_profile_buffer_head].thread_id
        = active_isr ? active_isr : ____cyg_get_thread_id();
    __cyg_profile_buffer[__cyg_profile_buffer_head].timestamp = ____cyg_get_timestamp();
    __cyg_profile_buffer[__cyg_profile_buffer_head].this_fn   = this_fn;
    __cyg_profile_buffer[__cyg_profile_buffer_head].call_site = call_site;
    __cyg_profile_buffer[__cyg_profile_buffer_head].is_enter
        = (is_enter << 0) | ((!!active_isr) << 1);
    __cyg_profile_buffer_head = (__cyg_profile_buffer_head + 1) % __cyg_buffer_size;
    cyg_flush_block_uart();
}

#pragma weak __cyg_profiler_init_flush = __cyg_profiler_init_uart
#pragma weak __cyg_profiler_flush      = __cyg_profiler_flush_uart
#pragma weak __cyg_profiler_store      = __cyg_profiler_store_uart