#pragma once
#include <stdint.h>

/** Board configs for STM32F411 Blackpill */

#define CFG_LED_RCC_PORT RCC_GPIOC
#define CFG_LED_PORT GPIOC
#define CFG_LED_PIN GPIO13

/**
 * Setup clocks for the device. See board.c for details
*/
void setupClocks(void);

/**
 * Setup pins used for GPIO
*/
void setupGpio(void);

/**
 * Setup usb as serial port
*/
void setupUsb(void);

/**
 * Setup logging
*/
void setupLog(void);

/**
 * Setup heartbeat task
*/
void setupHeartbeat(void);

/** threadsafe logging call */
uint32_t th_log(const char *str, uint32_t len);
