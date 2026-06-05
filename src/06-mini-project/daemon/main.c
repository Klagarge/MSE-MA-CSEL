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

#include "timer/timer.h"
#include "gpio/led.h"
#include "gpio/button.h"

#define DEFAULT_TIME_MS 1000
#define DUTY_CYCLE_PERCENT 2


typedef struct {
    atomic_int flash_period_ms;
    int timer_fd;
    int epoll_fd;
} ThreadData;

ThreadData data;

void period_inc() {
    int period = atomic_fetch_add(&data.flash_period_ms, 100);
    printf("period_inc: flash_period_ms=%d\n", period + 100);
}

void period_dec() {
    int period = atomic_fetch_sub(&data.flash_period_ms, 100);
    printf("period_dec: flash_period_ms=%d\n", period - 100);
}

void period_reset() {
    atomic_store(&data.flash_period_ms, DEFAULT_TIME_MS);
    printf("period_reset: flash_period_ms=%d\n", DEFAULT_TIME_MS);
}

static void* timer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    led_t* led = led_init(LED_POWER);
    led_off(led);

    struct epoll_event ev;

    int isLedOn = 0;
    while(1) {

        int n = epoll_wait(data->epoll_fd, &ev, 1, -1);
        if (n == -1) {
            perror("epoll_wait failed");
            break;
        }

        uint64_t val;
        if (read(data->timer_fd, &val, sizeof(val)) != sizeof(val)) {
            perror("read timerfd failed");
            break;
        }

        int period = atomic_load(&data->flash_period_ms);

        long time_on_ms = period / 100 * DUTY_CYCLE_PERCENT; // 2% duty
        long time_off_ms = period - time_on_ms; // rest of the period

        int delay = 0;
        if (isLedOn == 0) {
            delay = time_on_ms; // 2% duty
            led_on(led);
            isLedOn = 1;
        } else {
            delay = time_off_ms; // rest of the period
            led_off(led);
            isLedOn = 0;
        }

        timer_set_time(&data->timer_fd, delay);

    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;

    atomic_store(&data.flash_period_ms, DEFAULT_TIME_MS);

    // Create timerfd
    data.timer_fd = timer_create_empty();
    timer_set_time(&data.timer_fd, data.flash_period_ms);

    // Create epoll instance for the timer
    data.epoll_fd = epoll_create1(0);
    if (data.epoll_fd == -1) {
        perror("ERROR while create epoll");
        exit(20);
    }
    timer_link_to_epoll(&data.timer_fd, &data.epoll_fd);

    btn_t* btn_inc = btn_init(BTN_INCREASE);
    btn_t* btn_dec = btn_init(BTN_DECREASE);
    btn_t* btn_mode = btn_init(BTN_MODE);

    btn_set_callback(btn_inc, period_inc);
    btn_set_callback(btn_dec, period_dec);
    btn_set_callback(btn_mode, period_reset);


    if (pthread_create(&thread, NULL, timer_thread, &data) != 0) {
        perror("Failed to create timer thread");
        exit(30);
    }

    while (1) {
        sleep(1);
    }

    closelog();
    return 0;
}
