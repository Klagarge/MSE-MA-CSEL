/**
 * Copyright 2018 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose: NanoPi silly status led control system
 *
 * Autĥor:  Daniel Gachet
 * Date:    07.11.2018
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>  

#include "timer.h"

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED      "/sys/class/gpio/gpio10"
#define LED           "10"
#define DEFAULT_TIME_MS 1000
#define DUTY_CYCLE_PERCENT 2

typedef struct {
    long flash_period_ms;
    int timer_fd;
    int epoll_fd;
} ThreadData;

static int open_led() {
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // config pin
    f = open(GPIO_LED "/direction", O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    f = open(GPIO_LED "/value", O_RDWR);
    return f;
}

void led_on(int led) {
    pwrite(led, "1", sizeof("1"), 0);
}

void led_off(int led) {
    pwrite(led, "0", sizeof("0"), 0);
}

static void* timer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    int led = open_led();
    led_off(led);

    
    long time_on_ms = data->flash_period_ms / 100 * DUTY_CYCLE_PERCENT; // 2% duty
    long time_off_ms = data->flash_period_ms - time_on_ms; // rest of the period

    struct epoll_event ev;
    
    int isLedOn = 0;
    int k = 0;
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


        int delay = 0;
        if (isLedOn == 0) {
            delay = time_on_ms; // 2% duty
            led_on(led);
            printf("ping %d\n", k++);
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
    ThreadData data;
    pthread_t thread;

    data.flash_period_ms = DEFAULT_TIME_MS;

    // Create timerfd
    data.timer_fd = timer_create_empty();
    timer_set_time(&data.timer_fd, data.flash_period_ms);

    // Create epoll instance
    data.epoll_fd = epoll_create1(0);
    if (data.epoll_fd == -1) {
        perror("ERROR while create epoll");
        exit(20);
    }

    timer_link_to_epoll(&data.timer_fd, &data.epoll_fd);


    if (pthread_create(&thread, NULL, timer_thread, &data) != 0) {
        perror("Failed to create timer thread");
        exit(30);
    }

    pthread_join(thread, NULL);

    return 0;
}