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

#define MAX_BTN 10
BTN* btn_list[MAX_BTN];

pthread_mutex_t btn_list_mutex = PTHREAD_MUTEX_INITIALIZER;

int epoll_fd;
struct epoll_event ev[MAX_BTN];
atomic_int btn_tail = 0;
pthread_t epoll_thread_id;

void BTN_add_epoll_event(BTN* btn, int tail);
void epoll_init();
static void* epoll_thread(void* arg);

int BTN_init(BTN* btn, BTN_type type) {
    if (btn == NULL) return -1;

    pthread_mutex_init(&btn->mutex, NULL);
    btn->callback = NULL;

    char gpio_path[32] = GPIO_BTN_BASE;
    switch (type) {
        case BTN_INCREASE:
            strcpy(btn->pin, GPIO_BTN_INCREASE);
            break;
        case BTN_DECREASE:
            strcpy(btn->pin, GPIO_BTN_DECREASE);
            break;
        case BTN_MODE:
            strcpy(btn->pin, GPIO_BTN_MODE);
            break;
        default:
            printf("Invalid button type\n");
            return -1;
    }
    strcat(gpio_path, btn->pin);

    int f = open(GPIO_UNEXPORT, O_WRONLY);
    if (f != -1) {
        write(f, btn->pin, strlen(btn->pin));
        close(f);
    }

    f = open(GPIO_EXPORT, O_WRONLY);
    if (f == -1) return -1;
    write(f, btn->pin, strlen(btn->pin));
    close(f);

    char direction_path[100];
    strcpy(direction_path, gpio_path);
    strcat(direction_path, "/direction");

    f = open(direction_path, O_WRONLY);
    if (f == -1) return -1;
    write(f, "in", 2);
    close(f);

    char edge_path[100];
    strcpy(edge_path, gpio_path);
    strcat(edge_path, "/edge");

    f = open(edge_path, O_WRONLY);
    if (f == -1) return -1;
    write(f, "both", 4);
    close(f);

    char value_path[100];
    strcpy(value_path, gpio_path);
    strcat(value_path, "/value");

    f = open(value_path, O_RDONLY);
    if (f == -1) {
        printf("Failed to setup button on pin %s\n", btn->pin);
        return -1;
    }
    btn->fd = f;

    // Dummy read to clear initial state before waiting
    char buf[2];
    pread(btn->fd, buf, sizeof(buf), 0);

    pthread_mutex_lock(&btn_list_mutex);
    int tail = atomic_fetch_add(&btn_tail, 1);
    if (tail >= MAX_BTN) {
        pthread_mutex_unlock(&btn_list_mutex);
        perror("Failed to add epoll event");
        close(btn->fd);
        return -1;
    }

    btn_list[tail] = btn;
    if (tail == 0) {
        epoll_init();
    }
    BTN_add_epoll_event(btn, tail);
    pthread_mutex_unlock(&btn_list_mutex);

    return 0;
}

void BTN_deinit(BTN* btn) {
    if (btn == NULL) return;
    pthread_mutex_lock(&btn->mutex);
    if (btn->fd != -1) {
        close(btn->fd);
        btn->fd = -1;
    }
    pthread_mutex_unlock(&btn->mutex);
    pthread_mutex_destroy(&btn->mutex);
}

void BTN_set_callback(BTN* btn, BTN_callback callback) {
    pthread_mutex_lock(&btn->mutex);
    btn->callback = callback;
    pthread_mutex_unlock(&btn->mutex);
}

void BTN_add_epoll_event(BTN* btn, int tail) {

    // EPOLLIN is working well as EPOLLPRI (which is more used for priority data)
    // EPOLLERR is used to detect if there is an error
    // EPOLLET is for edge triggered mode (non-blocking)
    ev[tail].events = EPOLLIN | EPOLLERR | EPOLLET;
    ev[tail].data.ptr = btn;

    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, btn->fd, &ev[tail]);
    if (ret < 0) {
        perror("Failed to add epoll event");
    }
}

void epoll_init(){
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("Failed to create epoll instance");
        exit(EXIT_FAILURE);
    }
    epoll_fd = epfd;

    if (pthread_create(&epoll_thread_id, NULL, epoll_thread, NULL) != 0) {
        perror("Failed to create timer thread");
        exit(30);
    }
}

static void* epoll_thread(void* arg) {
    (void) arg;
    while (1) {
        struct epoll_event events[MAX_BTN];
        int n = epoll_wait(epoll_fd, events, MAX_BTN, -1);
        if (n < 0) {
            perror("epoll_wait");
            continue;
        }
        for (int i = 0; i < n; i++) {
            char buf[2];
            BTN* btn = (BTN*)events[i].data.ptr;
            if (btn == NULL) continue;

            pthread_mutex_lock(&btn->mutex);
            if (btn->fd != -1) {
                pread(btn->fd, buf, sizeof(buf), 0);
                if (buf[0] == '1') {
                    if (btn->callback != NULL) {
                        btn->callback();
                    }
                }
            }
            pthread_mutex_unlock(&btn->mutex);
        }
    }
    return NULL;
}
