#import "/doc/metadata.typ": *

= Linux System Programming

== File system
This laboratory is focused on the user space. We make an application to change the blinking frequency of a led with button.

=== Buttons

// TODO

It has some difficulties to locate the button on the GPIOs. Because this is not
the same as we have done in the module in the precedent laboratory.
-> explain

Discover the epoll to trigg on file modification

discover the timer_fd

all is file

all can be in the epoll to manage all event from files


=== timer
- epoll
- thread
- 1 consumer, 1 provider, no need mutex

- create timer empty
- set timer with `timerfd_set_time`
- definition of the initial time and not the repetition time, to use only one timer
- link to epoll


=== syslog
- one opening
```c openlog("CSEL Logs", LOG_PID, LOG_USER);```
LOG_PID is used to keep the PID of the process in the log, and LOG_USER is used to specify the facility of the log (what type of programme).

```c syslog(LOG_INFO, "Start logging silly led-controller"); // INFO level```


=== Multithreading
- one thread for the button, one for the led
- button write time
- led read time and sleep for this time
- no mutex because we have only one provider