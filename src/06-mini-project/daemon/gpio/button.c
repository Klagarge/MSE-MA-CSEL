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

int epoll_fd;
struct epoll_event ev[MAX_BTN];
atomic_int btn_tail = 0;
pthread_t epoll_thread_id;

void BTN_add_epoll_event(BTN* btn, int tail);
void epoll_init();
static void* epoll_thread(void* arg);

BTN* BTN_init(BTN_type type) {
    BTN* btn = malloc(sizeof(BTN));
    if (btn == NULL) return NULL;

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
            return NULL;
    }
    strcat(gpio_path, btn->pin);

    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, btn->pin, strlen(btn->pin));
    close(f);

    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, btn->pin, strlen(btn->pin));
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
        printf("Failed to setup button on pin %s\n", btn->pin);
        return NULL;
    }
    btn->fd = f;

    // Dummy read to clear initial state before waiting
    char buf[2];
    pread(btn->fd, buf, sizeof(buf), 0);

    int tail = atomic_fetch_add(&btn_tail, 1);
    if (tail >= MAX_BTN) {
        perror("Failed to add epoll event");
        exit(EXIT_FAILURE);
    }

    btn_list[tail] = btn;
    if (tail == 0) {
        epoll_init();
    }
    BTN_add_epoll_event(btn, tail);

    return btn;
}

void BTN_set_callback(BTN* btn, BTN_callback callback) {
    btn->callback = callback;
}

// TODO add mutex to protect this function
void BTN_add_epoll_event(BTN* btn, int tail) {

    // EPOLLIN is working well as EPOLLPRI (which is more used for priority data)
    // EPOLLERR is used to detect if there is an error
    // EPOLLET is for edge triggered mode (non-blocking)
    ev[tail].events = EPOLLIN | EPOLLERR | EPOLLET;
    ev[tail].data.fd = btn->fd;

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
            int fd = events[i].data.fd;
            int tail = -1;
            BTN* btn = NULL;
            for (int j = 0; j < MAX_BTN; j++) {
                if (btn_list[j] == NULL) continue;
                if (btn_list[j]->fd == fd) {
                    tail = j;
                    btn = btn_list[j];
                    break;
                }
            }
            if (tail == -1) {
                printf("No button found for fd %d\n", fd);
                continue;
            }
            pread(btn->fd, buf, sizeof(buf), 0);
            if (buf[0] == '1') {
                // printf("Button %s pressed\n", btn->pin);
                if (btn->callback != NULL) {
                    btn->callback();
                }
            }
        }
    }
    return NULL;
}
