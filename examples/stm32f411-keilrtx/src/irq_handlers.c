
#include <stdint.h>

extern void SVC_Handler();
extern void PendSV_Handler();
extern void SysTick_Handler();

/**
 * Route libopencm3 irq handlers to rtx handlers
 */
void sv_call_handler(void) { SVC_Handler(); }
void pend_sv_handler(void) { PendSV_Handler(); }
void sys_tick_handler(void) { SysTick_Handler(); }
