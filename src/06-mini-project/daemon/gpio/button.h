#ifndef BUTTON_H
#define BUTTON_H

#define GPIO_BTN_INCREASE "0"
#define GPIO_BTN_DECREASE "2"
#define GPIO_BTN_MODE "3"

typedef enum {
    BTN_INCREASE,
    BTN_DECREASE,
    BTN_MODE,
} btn_type_t;

typedef void (*btn_callback_t)();

typedef struct {
    int gpio;
    btn_callback_t callback;
} btn_t;

btn_t* btn_init(btn_type_t type);
void btn_set_callback(btn_t* btn, btn_callback_t callback);

#endif
