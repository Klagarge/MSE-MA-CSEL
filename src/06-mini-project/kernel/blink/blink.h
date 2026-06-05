#ifndef BLINK_H
#define BLINK_H

#include <linux/module.h>	// needed by all modules
#include <linux/init.h>		// needed for macros
#include <linux/kernel.h>	// needed for debugging


#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/gpio.h>

#include <linux/atomic.h>


#define GPIO_PIN 10
#define DEFAULT_PERIOD_MS 1000

/**
 * Allow to set a new period for the blinky led
 * @param new_period_ms The new period in ms for the blinky led
 */
void adjust_period(int new_period_ms);

void blink_init(void);
void blink_exit(void);

#endif /* BLINK_H */
