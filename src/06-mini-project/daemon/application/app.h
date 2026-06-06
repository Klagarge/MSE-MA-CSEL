#ifndef APP_H
#define APP_H

#include <stdint.h>

#include "../gpio/led.h"

#define DEFAULT_PERIOD_MS 1000

#define GAP_PERIOD_MS 50

/* --- Period functions --- */
void increase_period();
void decrease_period();
void set_period(uint32_t period_ms);
uint32_t get_period();


/* --- Mode functions --- */
void mode_toggle();
void set_mode(int mode);
int get_mode();


/* --- Temperature functions --- */
float get_temperature();


/* --- Button specific function --- */

void btn_set_led(LED* l);
void init_animations(void);

void btn_increase_period();
void btn_decrease_period();

#endif /* APP_H */
