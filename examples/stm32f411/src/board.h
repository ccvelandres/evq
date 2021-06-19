#pragma once

#define LED_RCC_PORT RCC_GPIOC
#define LED_PORT GPIOC
#define LED_PIN GPIO13

/**
 * Setup clocks for the device. See board.c for details
*/
void setupClocks();

/**
 * Setup pins used for GPIO
*/
void setupGpio();
