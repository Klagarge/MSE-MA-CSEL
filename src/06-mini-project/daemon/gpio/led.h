#ifndef LED_H
#define LED_H

#include <pthread.h>

#define GPIO_LED_STATUS "10"
#define GPIO_LED_POWER  "362"

typedef enum {
    LED_STATUS, // gpioa.10 --> gpio10
    LED_POWER,  // gpiol.10 --> gpio362
} LED_type;

typedef struct {
    int gpio;
    pthread_mutex_t mutex;
} LED;

int LED_init(LED* led, LED_type type);
void LED_on(LED* led);
void LED_off(LED* led);
void LED_toggle(LED* led);
void LED_deinit(LED* led);

#endif //LED_H
