#include "profiler.h"

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/itm.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/stm32/rcc.h>

#define ITM_BAUDRATE 230400

void inline itm_put8(uint32_t channel, uint8_t ch)
{
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
        ;
    ITM_STIM8(channel) = ch;
}

void inline itm_put32(uint32_t channel, uint32_t ch)
{
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
        ;
    ITM_STIM32(channel) = ch;
}

void cyg_profiler_init_itm()
{
    // configure trace swo
    rcc_periph_clock_enable(RCC_GPIOB);

    SCS_DEMCR = SCS_DEMCR_TRCENA; // enable trace
    DBGMCU_CR |= DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_ASYNC;

    // use ahb assuming sysclk == ahb freq
    TPIU_ACPR  = ((rcc_ahb_frequency + ITM_BAUDRATE / 2) / ITM_BAUDRATE);
    TPIU_CSPSR = 1;
    TPIU_SPPR  = TPIU_SPPR_ASYNC_MANCHESTER; // Use manchester encoding
    TPIU_FFCR  = 0;                          // disable formatting

    // Unlock ITM
    ITM_LAR    = CORESIGHT_LAR_KEY;
    ITM_TCR    = ITM_TCR_ITMENA | ITM_TCR_SYNCENA | (1 << 16);
    ITM_TPR    = 0xF;  // enable unpriv access
    ITM_TER[0] = 0xFF; // enable 0:7 stimulus ports

    // configure dwt if needed
}

bool cyg_profiler_flush_itm(void *ptr, uint16_t len)
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

void cyg_profiler_store_itm(unsigned int is_enter, void *this_fn, void *call_site)
{
    // we don't really store entries with itm, can just push to fifo
    unsigned int active_isr = cyg_get_active_isr();
    itm_put32(0, active_isr ? active_isr : cyg_get_thread_id());
    itm_put32(0, cyg_get_timestamp());
    itm_put32(0, (uint32_t) this_fn);
    itm_put32(0, (uint32_t) call_site);
    itm_put32(0, (is_enter << 0) | ((!!active_isr) << 1));
}

#pragma weak cyg_profiler_init_flush = cyg_profiler_init_itm
#pragma weak cyg_profiler_flush      = cyg_profiler_flush_itm
#pragma weak cyg_profiler_store      = cyg_profiler_store_itm