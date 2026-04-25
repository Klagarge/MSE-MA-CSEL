#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


int timer_create_empty() {

    // Create timerfd
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd == -1) {
        perror("timerfd_create failed");
        exit(10);
    }

    return timer_fd;
}


void timer_set_time(int* timer_fd, long period_ms) {
    // https://www.man7.org/linux/man-pages/man3/itimerspec.3type.html
    struct itimerspec its;
    
    // Periodic interval
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    
    // Initial expiration
    its.it_value.tv_sec = period_ms / 1000;
    its.it_value.tv_nsec = (period_ms % 1000) * 1000000;

    if (timerfd_settime(*timer_fd, 0, &its, NULL) == -1) {
        perror("timerfd_settime failed");
        exit(11);
    }
}

void timer_link_to_epoll(int* timer_fd, int* epoll_fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = *timer_fd;
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *timer_fd, &ev) == -1) {
        perror("ERROR while add timerfd to epoll");
        exit(21);
    }
}