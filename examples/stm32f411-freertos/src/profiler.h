#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct __profile_entry
{
    unsigned int is_enter;
    unsigned int thread_id;
    unsigned int timestamp;
    void        *this_fn;
    void        *call_site;
} __cyg_profile_entry_t;

void __cyg_profiler_init(void);
void __cyg_profiler_start(void);
void __cyg_profiler_end(void);

// Info function for profiler
#define __rtos_freertos 1

#if defined(__rtos_freertos)
#include <FreeRTOS.h>
extern void               *pxCurrentTCB;
static inline unsigned int __cyg_get_thread_id(void) { return (unsigned int)pxCurrentTCB; }
#else
static inline unsigned int __cyg_get_thread_id(void) { return 0; }
#endif
#define US_PER_TICK 10
static volatile uint32_t   __cyg_timestamp; // Incremented by profiler timebase
static inline unsigned int __cyg_get_timestamp(void) { return __cyg_timestamp; }

static inline unsigned int __cyg_get_active_isr(void)
{
    // test get active vector id from SCB->ICSR
    const uint32_t *__scb                     = (uint32_t *)0xE000ED04;
    const uint32_t  __scb_icsr_vectactive_msk = 0xFF;
    return ((*__scb) & __scb_icsr_vectactive_msk);
}

// Stream dependent functions
bool __cyg_profiler_flush(void *ptr, uint16_t len);
void __cyg_profiler_init_flush();
void __cyg_profiler_store(unsigned int is_enter, void *this_fn, void *call_site);