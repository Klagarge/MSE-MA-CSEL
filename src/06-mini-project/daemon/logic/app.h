#ifndef APP_H
#define APP_H

#include "../gpio/led.h"

#define DEFAULT_PERIOD_MS 1000

void set_led(

void increase_period();
void decrease_period();
void set_period(int period_ms);

void mode_toggle();
void set_mode(int mode);

float get_temperature();


/*  Button specific function */

void btn_increase_period();
void btn_decrease_period();

#endif /* APP_H */
