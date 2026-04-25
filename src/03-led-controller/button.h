#ifndef BUTTON_H
#define BUTTON_H

#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

int btn_open(const char* gpio_path, const char* pin);


#endif
