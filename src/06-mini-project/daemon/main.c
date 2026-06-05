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

#include "timer/timer.h"
#include "gpio/led.h"
#include "gpio/button.h"



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


void* btn_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    btn_init(BTN_INCREASE);
    btn_init(BTN_DECREASE);
    btn_init(BTN_MODE);


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
                    data->flash_period_ms += 200;
                    char* log_msg = malloc(100);
                    snprintf(log_msg, 100, "Increase flash period to %ld ms", data->flash_period_ms);
                    syslog(LOG_INFO, "%s", log_msg);
                    printf("%s\n", log_msg);
                    free(log_msg);
                }

            } else if (events[i].data.fd == btn[1]) {
                if (buf[0] == '1') {
                    data->flash_period_ms = DEFAULT_TIME_MS;
                    char* log_msg = malloc(100);
                    snprintf(log_msg, 100, "Reset flash period to %ld ms", data->flash_period_ms);
                    syslog(LOG_INFO, "%s", log_msg);
                    printf("%s\n", log_msg);
                    free(log_msg);
                }

            } else if (events[i].data.fd == btn[2]) {
                if (buf[0] == '1') {
                    data->flash_period_ms -= 200;
                    if (data->flash_period_ms <= 0) {
                        data->flash_period_ms = 200; // Minimum period of 200ms
                    }
                    char* log_msg = malloc(100);
                    snprintf(log_msg, 100, "Decrease flash period to %ld ms", data->flash_period_ms);
                    syslog(LOG_INFO, "%s", log_msg);
                    printf("%s\n", log_msg);
                    free(log_msg);
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

        long time_on_ms = data->flash_period_ms / 100 * DUTY_CYCLE_PERCENT; // 2% duty
        long time_off_ms = data->flash_period_ms - time_on_ms; // rest of the period

        int delay = 0;
        if (isLedOn == 0) {
            delay = time_on_ms; // 2% duty
            // led_on(led);
            led_toggle(led);
            isLedOn = 1;
        } else {
            delay = time_off_ms; // rest of the period
            // led_off(led);
            led_toggle(led);
            isLedOn = 0;
        }

        timer_set_time(&data->timer_fd, delay);

    }
    return NULL;
}

int main(int argc, char* argv[]) {
    ThreadData data;
    pthread_t thread;
    openlog("CSEL Logs", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start logging silly led-controller");

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


    // Setup button thread
    pthread_t btn_thread_inst;
    pthread_create(&btn_thread_inst, NULL, btn_thread, &data);

    while (1) {
        sleep(1);
    }

    closelog();
    return 0;
}
