#include "button.h"

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>


#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

int btn_open(const char* gpio_path, const char* pin) {
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
    return f;
}
