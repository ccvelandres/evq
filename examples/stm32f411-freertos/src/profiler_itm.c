#include "profiler.h"

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/itm.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/stm32/rcc.h>

#define ITM_BAUDRATE 2000000

__attribute__((no_instrument_function)) void inline itm_put8(uint32_t channel, uint8_t ch)
{
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
        ;
    ITM_STIM8(channel) = ch;
}

__attribute__((no_instrument_function)) void inline itm_put32(uint32_t channel, uint32_t ch)
{
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
        ;
    ITM_STIM32(channel) = ch;
}

__attribute__((no_instrument_function)) void __cyg_profiler_init_itm()
{
    // configure trace swo
    rcc_periph_clock_enable(RCC_GPIOB);

    SCS_DEMCR = SCS_DEMCR_TRCENA; // enable trace
    DBGMCU_CR |= DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_ASYNC;

    // use ahb assuming sysclk == ahb freq
    TPIU_ACPR  = ((rcc_ahb_frequency + ITM_BAUDRATE / 2) / ITM_BAUDRATE);
    TPIU_CSPSR = 1;                   // port size: 1 bit
    TPIU_SPPR  = TPIU_SPPR_ASYNC_NRZ; // Use manchester encoding
    TPIU_FFCR  = 0x100;               // disable formatting

    // Unlock ITM
    ITM_LAR    = CORESIGHT_LAR_KEY;
    ITM_TCR    = ITM_TCR_ITMENA | ITM_TCR_SYNCENA | (1 << 16);
    ITM_TPR    = 0xF;  // enable unpriv access
    ITM_TER[0] = 0x3F; // enable stimulus ports 0

    // configure dwt if needed
    DWT_CTRL = 0x400003FE; // disable sync packets
}

__attribute__((no_instrument_function)) bool __cyg_profiler_flush_itm(void *ptr, uint16_t len)
{
    uint8_t *data      = ptr;
    uint8_t *end       = ((uint8_t *)ptr) + len;
    uint32_t rem_bytes = 0;

    while ((rem_bytes = (end - data)) > 0)
    {
        if (rem_bytes >= 4)
        {
            itm_put32(0, *(uint32_t *)data);
            data += 4;
        }
        else
        {
            itm_put32(0, *(uint32_t *)data);
            data += 1;
        }
    }
    return true;
}

__attribute__((no_instrument_function)) void __cyg_profiler_store_itm(unsigned int is_enter,
                                                                      void        *this_fn,
                                                                      void        *call_site)
{
    unsigned int active_isr = __cyg_get_active_isr();
    // we don't really store entries with itm, can just push to fifo

    // TSDL header for 16 bit packet header [0] = enter, [1] = exit
    static const uint32_t header[2] = {0x00001fc1, 0x00011fc1};
    itm_put32(0, header[is_enter ? 1 : 0]);

    // Push sequence should follow struct format
    // 1. Flags
    itm_put32(1, (is_enter << 0) | ((!!active_isr) << 1));
    // 2. Thread_id
    itm_put32(2, active_isr ? active_isr : __cyg_get_thread_id());
    // 3. timestamp
    itm_put32(3, __cyg_get_timestamp());
    // 4. this_fn
    itm_put32(4, (uint32_t)this_fn);
    // 4. call_site
    itm_put32(5, (uint32_t)call_site);
}

#pragma weak __cyg_profiler_init_flush = __cyg_profiler_init_itm
#pragma weak __cyg_profiler_flush      = __cyg_profiler_flush_itm
#pragma weak __cyg_profiler_store      = __cyg_profiler_store_itm