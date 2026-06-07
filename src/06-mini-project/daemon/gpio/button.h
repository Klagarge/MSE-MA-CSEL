#ifndef BUTTON_H
#define BUTTON_H

#include <pthread.h>

#define GPIO_BTN_INCREASE "3"
#define GPIO_BTN_DECREASE "0"
#define GPIO_BTN_MODE "2"

typedef enum {
    BTN_INCREASE,
    BTN_DECREASE,
    BTN_MODE,
} BTN_type;

typedef void (*BTN_callback)();

typedef struct {
    int fd;
    char pin[32];
    BTN_callback callback;
    pthread_mutex_t mutex;
} BTN;

BTN* BTN_init(BTN_type type);
void BTN_set_callback(BTN* btn, BTN_callback callback);

#endif
