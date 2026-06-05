#ifndef CSEL_WORKSPACE_TIMER_H
#define CSEL_WORKSPACE_TIMER_H

int timer_create_empty();
void timer_set_time(int* timer_fd, long period_ms);
void timer_link_to_epoll(int* timer_fd, int* epoll_fd);

#endif //CSEL_WORKSPACE_TIMER_H
