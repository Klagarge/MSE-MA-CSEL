#include "button.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_BTN_BASE "/sys/class/gpio/gpio"

#define MAX_EVENTS 10
atomic_int ev_tail = 0;

int epoll_fd;
struct epoll_event ev[MAX_EVENTS];
pthread_t epoll_thread_id;

int btn_add_epoll_event(btn_t* btn);
void epoll_init();
static void* epoll_thread(void* arg);

btn_t* btn_init(btn_type_t type) {
    btn_t* btn = malloc(sizeof(btn_t));
    if (btn == NULL) return NULL;

    char gpio_path[32] = GPIO_BTN_BASE;
    char pin[32];
    switch (type) {
        case BTN_INCREASE:
            strcpy(pin, GPIO_BTN_INCREASE);
            break;
        case BTN_DECREASE:
            strcpy(pin, GPIO_BTN_DECREASE);
            break;
        case BTN_MODE:
            strcpy(pin, GPIO_BTN_MODE);
            break;
        default:
            printf("Invalid button type\n");
            return NULL;
    }
    strcat(gpio_path, pin);

    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    char direction_path[100];
    strcpy(direction_path, gpio_path);
    strcat(direction_path, "/direction");

    f = open(direction_path, O_WRONLY);
    write(f, "in", 2);
    close(f);

    char edge_path[100];
    strcpy(edge_path, gpio_path);
    strcat(edge_path, "/edge");

    f = open(edge_path, O_WRONLY);
    write(f, "both", 4); // "both" means it triggers on press AND release
    close(f);

    char value_path[100];
    strcpy(value_path, gpio_path);
    strcat(value_path, "/value");

    f = open(value_path, O_RDONLY);
    if (f == -1) {
        printf("Failed to setup button on pin %s\n", pin);
        return NULL;
    }
    btn->gpio = f;

    // Dummy read to clear initial state before waiting
    char buf[2];
    pread(btn->gpio, buf, sizeof(buf), 0);

    btn_add_epoll_event(btn);

    return btn;
}

void btn_set_callback(btn_t* btn, btn_callback_t callback) {
    btn->callback = callback;
}

// TODO add mutex to protect this function
int btn_add_epoll_event(btn_t* btn) {
    int tail = atomic_fetch_add(&ev_tail, 1);
    if (tail >= MAX_EVENTS-1) {
        perror("Failed to add epoll event");
        return -1;
    }

    if (tail == 0) {
        epoll_init();
    }

    // EPOLLIN is working well as EPOLLPRI (which is more used for priority data)
    // EPOLLERR is used to detect if there is an error
    // EPOLLET is for edge triggered mode (non-blocking)
    ev[tail].events = EPOLLIN | EPOLLERR | EPOLLET;
    ev[tail].data.fd = btn->gpio;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, btn->gpio, &ev[tail]);
    if (ret < 0) {
        perror("Failed to add epoll event");
        return -1;
    }
    return tail;
}

void epoll_init(){
    if (pthread_create(&epoll_thread_id, NULL, epoll_thread, NULL) != 0) {
        perror("Failed to create timer thread");
        exit(30);
    }
}

static void* epoll_thread(void* arg) {
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("Failed to create epoll instance");
        exit(EXIT_FAILURE);
    }
    epoll_fd = epfd;

    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("epoll_wait");
            continue;
        }
        for (int i = 0; i < n; i++) {

        }
    }
}
