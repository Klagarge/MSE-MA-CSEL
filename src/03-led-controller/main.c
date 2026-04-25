#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <pthread.h>

#include "setup.c"
#include "timer.h"

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */

#define GPIO_LED       "/sys/class/gpio/gpio10"
#define LED            "10"

#define GPIO_BTN1      "/sys/class/gpio/gpio0"
#define BTN1           "0"
#define GPIO_BTN2      "/sys/class/gpio/gpio2"
#define BTN2           "2"
#define GPIO_BTN3      "/sys/class/gpio/gpio3"
#define BTN3           "3"

#define NBR_BTN 3

#define DEFAULT_TIME_MS 1000
#define DUTY_CYCLE_PERCENT 2


typedef struct {
    long flash_period_ms;
    int timer_fd;
    int epoll_fd;
} ThreadData;

// constant
const char* GPIO_BTN[NBR_BTN] = {GPIO_BTN1, GPIO_BTN2, GPIO_BTN3};
const char* BTN[NBR_BTN] =  {BTN1, BTN2, BTN3};


void led_on(int led) {
    pwrite(led, "1", sizeof("1"), 0);
}

void led_off(int led) {
    pwrite(led, "0", sizeof("0"), 0);
}

void* btn_thread(void* arg) {
    // Open all button with the right flags
    int btn[NBR_BTN] = {0};
    for(int i=0; i<NBR_BTN; i++) {
        btn[i] = open_btn(GPIO_BTN[i], BTN[i]);
        if (btn[i] < 0) {
            perror("Failed to open button");
        }
    }

    // Create epoll instance to control all button files
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("Failed to create epoll");
    }

    // Add buttons to epoll
    struct epoll_event ev[NBR_BTN];
    // EPOLLIN is working well as EPOLLPRI (which is more used for priority data)
    // EPOLLERR is used to detect if there is an error
    // EPOLLET is for edge triggered mode (non-blocking)
    for(int i=0; i<NBR_BTN; i++) {
        ev[i].events = EPOLLIN | EPOLLERR | EPOLLET;
        ev[i].data.fd = btn[i];
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, btn[i], &ev[i]) < 0) {
            perror("Failed to add button to epoll");
        }
    }

    // Dummy read to clear initial state before waiting
    char buf[2];
    for(int i=0; i<NBR_BTN; i++) {
        pread(btn[i], buf, sizeof(buf), 0);
    }

    printf("Waiting for button presses...\n");

    // Event main loop
    while (1) {
        struct epoll_event events[NBR_BTN];

        // Timeout is -1: Block infinitely until an event occurs!
        int n = epoll_wait(epfd, events, 1, -1);

        if (n < 0) {
            perror("epoll_wait error");
            break;
        }

        for (int i = 0; i < n; i++) {
            // read btn file
            pread(events[i].data.fd, buf, sizeof(buf), 0);

            if (events[i].data.fd == btn[0]) {
                if (buf[0] == '1') {
                    printf("Decrease led frequency\n");
                }

            } else if (events[i].data.fd == btn[1]) {
                if (buf[0] == '1') {
                    printf("Reset led frequency\n");
                }

            } else if (events[i].data.fd == btn[2]) {
                if (buf[0] == '1') {
                    printf("Increase led frequency\n");
                }
            }
        }
    }

    for (int i=0; i<NBR_BTN; i++) {
        close(btn[i]);
    }
    close(epfd);
}

static void* timer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    int led = open_led(GPIO_LED, LED);
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

int main(int argc, char* argv[])
{
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


// Yann -------------------
    // Setup button thread
    pthread_t btn_thread_inst;
    pthread_create(&btn_thread_inst, NULL, btn_thread, NULL);


    // pthread_join(btn_thread_inst, NULL);
    // pthread_join(thread, NULL);

    while (1) {
        sleep(1);
    }


    return 0;
}
