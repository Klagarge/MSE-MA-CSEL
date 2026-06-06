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
#include "oled/oled.h"
#include "application/app.h"

#define DEFAULT_TIME_MS 1000
#define DUTY_CYCLE_PERCENT 2


int main(void) {


    btn_t* btn_inc = btn_init(BTN_INCREASE);
    btn_t* btn_dec = btn_init(BTN_DECREASE);
    btn_t* btn_mode = btn_init(BTN_MODE);

    led_t* led_power = led_init(LED_POWER);

    btn_set_callback(btn_inc, btn_increase_period);
    btn_set_callback(btn_dec, btn_decrease_period);
    btn_set_callback(btn_mode, mode_toggle);

    struct ipc_callbacks_t ipc_cbs = {
        .on_dec_period = decrease_period,
        .on_inc_period = increase_period,
        .on_set_period = set_period,
        .on_set_mode = set_mode,
    };

    struct oled_callbacks_t oled_cbs = {
        .get_mode = get_mode,
        .get_period = get_period,
        .get_temperature = get_temperature,
    };

    int ret = start_ipc_server(&ipc_cbs);
    if (ret < 0) {
        fprintf(stderr, "Failed to start IPC server: %d\n", ret);
        return 1;
    }

    // init_oled(&oled_cbs);

    btn_set_led(led_power);
    init_animations();

    while (1) {
        sleep(1);
    }

    stop_ipc_server();
    return 0;
}
