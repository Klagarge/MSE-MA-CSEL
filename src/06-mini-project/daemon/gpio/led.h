#ifndef CSEL_WORKSPACE_LED_H
#define CSEL_WORKSPACE_LED_H

#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

#define GPIO_LED       "/sys/class/gpio/gpio10"
#define LED            "10"

int led_open(const char* gpio_path, const char* pin);
void led_on(int led);
void led_off(int led);

#endif //CSEL_WORKSPACE_LED_H
