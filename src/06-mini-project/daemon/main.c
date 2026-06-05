#include <fcntl.h>
#include <stdio.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <syslog.h>
#include <stdatomic.h>

#include "gpio/led.h"
#include "gpio/button.h"
#include "ipc/ipc_server.h"

#define DEFAULT_TIME_MS 1000
#define DUTY_CYCLE_PERCENT 2

void set_mode(int mode) {
    printf("set_mode: %d\n", mode);
}

void set_period(int period) {
    printf("set_period: %d\n", period);
}

uint32_t get_period() {
    printf("get_period\n");
    return 200;
}

void period_inc() {
    printf("period_inc\n");
}

void period_dec() {
    printf("period_dec\n");
}

void period_reset() {
    printf("period_reset\n");
}

void get_temp() {
    printf("get_temp\n");
}

int main(void) {


    btn_t* btn_inc = btn_init(BTN_INCREASE);
    btn_t* btn_dec = btn_init(BTN_DECREASE);
    btn_t* btn_mode = btn_init(BTN_MODE);

    led_t* led_power = led_init(LED_POWER);

    btn_set_callback(btn_inc, period_inc);
    btn_set_callback(btn_dec, period_dec);
    btn_set_callback(btn_mode, period_reset);

    struct ipc_callbacks_t ipc_cbs = {
        .on_dec_frequency = period_dec,
        .on_inc_frequency = period_inc,
        .on_set_frequency = set_period,
        .on_set_mode = set_mode,
    };

    int ret = start_ipc_server(&ipc_cbs);
    if (ret < 0) {
        fprintf(stderr, "Failed to start IPC server: %d\n", ret);
        return 1;
    }

    while (1) {
        sleep(1);
    }

    stop_ipc_server();
    return 0;
}
