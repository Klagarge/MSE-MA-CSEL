#ifndef LED_H
#define LED_H

#define GPIO_LED_STATUS "10"
#define GPIO_LED_POWER  "362"

typedef enum {
    LED_STATUS, // gpioa.10 --> gpio10
    LED_POWER,  // gpiol.10 --> gpio362
} led_type_t;

typedef struct {
    int gpio;
} led_t;

led_t* led_init(led_type_t type);
void led_on(led_t* led);
void led_off(led_t* led);
void led_toggle(led_t* led);

#endif //LED_H
