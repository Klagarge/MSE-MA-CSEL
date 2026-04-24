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

static void toggle_led(int led) {
    static int k = 0;
    if (k%2 == 0) {
        printf("ping %d\n", k>>1);
        pwrite(led, "1", sizeof("1"), 0);
    } else {
        pwrite(led, "0", sizeof("0"), 0);
    }
    k += 1;
}

static void* timer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    int led = open_led();
    pwrite(led, "1", sizeof("1"), 0);

    
    long flash_period_ms = DEFAULT_TIME_MS / 100 * DUTY_CYCLE_PERCENT; // 2% duty
    struct epoll_event ev;
    
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

        toggle_led(led);
        struct timespec t1, t2;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        t2.tv_sec = t1.tv_sec + flash_period_ms / 1000;
        t2.tv_nsec = t1.tv_nsec + (flash_period_ms % 1000) * 1000000;

        while(t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_nsec < t2.tv_nsec)) {
            clock_gettime(CLOCK_MONOTONIC, &t1);
        }
        
        toggle_led(led);
    }
    return NULL;
}

int create_timer(long period_ms) {

    // Create timerfd
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd == -1) {
        perror("timerfd_create failed");
        exit(10);
    }

    // Configure timer to expire every period_ms milliseconds
    // https://www.man7.org/linux/man-pages/man3/itimerspec.3type.html
    
    struct itimerspec its;
    
    // Periodic interval
    its.it_interval.tv_sec = period_ms / 1000;
    its.it_interval.tv_nsec = (period_ms % 1000) * 1000000;
    
    // Initial expiration with same value as periodic interval
    its.it_value.tv_sec = period_ms / 1000;
    its.it_value.tv_nsec = (period_ms % 1000) * 1000000;
    
    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        perror("timerfd_settime failed");
        exit(11);
    }

    return timer_fd;
}

void link_timer_to_epoll(int* timer_fd, int* epoll_fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = *timer_fd;
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *timer_fd, &ev) == -1) {
        perror("ERROR while add timerfd to epoll");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    ThreadData data;
    pthread_t thread;

    data.timer_fd = create_timer(DEFAULT_TIME_MS);

    // Create epoll instance
    data.epoll_fd = epoll_create1(0);
    if (data.epoll_fd == -1) {
        perror("ERROR while create epoll");
        exit(1);
    }

    link_timer_to_epoll(&data.timer_fd, &data.epoll_fd);


    if (pthread_create(&thread, NULL, timer_thread, &data) != 0) {
        perror("Failed to create timer thread");
        exit(1);
    }

    pthread_join(thread, NULL);

    return 0;
}