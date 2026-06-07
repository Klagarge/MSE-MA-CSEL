#include "led.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <stdio.h>

#define GPIO_EXPORT     "/sys/class/gpio/export"
#define GPIO_UNEXPORT   "/sys/class/gpio/unexport"
#define GPIO_LED_BASE   "/sys/class/gpio/gpio"

#include <stdlib.h>

int LED_init(LED* led, LED_type type) {
    if (led == NULL) return -1;

    // Concatenate GPIO LED path based on type
    char gpio_path[32] = GPIO_LED_BASE;
    char pin[32];
    switch (type) {
        case LED_STATUS:
            strcpy(pin, GPIO_LED_STATUS);
            break;
        case LED_POWER:
            strcpy(pin, GPIO_LED_POWER);
            break;
        default:
            printf("Invalid LED type\n");
            return -1;
    }
    strcat(gpio_path, pin);

    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    // config pin
    char direction_path[100];
    strcpy(direction_path, gpio_path);
    strcat(direction_path, "/direction");

    f = open(direction_path, O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    char value_path[100];
    strcpy(value_path, gpio_path);
    strcat(value_path, "/value");

    f = open(value_path, O_RDWR);
    if (f == -1) {
        printf("Failed to setup led on pin %s\n", pin);
        return -1;
    }

    led->gpio = f;
    if (pthread_mutex_init(&led->mutex, NULL) != 0) {
        close(f);
        return -1;
    }
    return 0;
}

void LED_on(LED* led) {
    if (led == NULL) {
        return;
    }
    pthread_mutex_lock(&led->mutex);
    pwrite(led->gpio, "1", sizeof("1"), 0);
    pthread_mutex_unlock(&led->mutex);
}

void LED_off(LED* led) {
    if (led == NULL) {
        return;
    }
    pthread_mutex_lock(&led->mutex);
    pwrite(led->gpio, "0", sizeof("0"), 0);
    pthread_mutex_unlock(&led->mutex);
}

void LED_toggle(LED* led) {
    if (led == NULL) {
        return;
    }
    pthread_mutex_lock(&led->mutex);
    char value[2];
    pread(led->gpio, value, sizeof(value), 0);
    if (value[0] == '0') {
        pwrite(led->gpio, "1", sizeof("1"), 0);
    } else {
        pwrite(led->gpio, "0", sizeof("0"), 0);
    }
    pthread_mutex_unlock(&led->mutex);
}

void LED_deinit(LED* led) {
    if (led == NULL) {
        return;
    }
    pthread_mutex_destroy(&led->mutex);
    close(led->gpio);
}
