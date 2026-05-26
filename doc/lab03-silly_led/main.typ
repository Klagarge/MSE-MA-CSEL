#import "/doc/metadata.typ": *

= Linux System Programming

This laboratory implements a user-space application for the NanoPi NEO Plus2 that controls the blinking frequency of the status LED using three push buttons. The main goal was to replace a CPU-intensive busy loop with an event-driven design.


== Design
The application is based on multithreading: one thread handles the LED timing, while another handles button events. GPIOs are accessed through sysfs, which allows the LED and buttons to be managed as file descriptors. A key design choice was to centralize all events with a single epoll instance, so both timer events and button events can be processed efficiently. 

All logs are done using the syslog at info level:
```c
syslog(LOG_INFO, "Start logging silly led-controller"); // INFO level
```

== Difficulties
The most difficult part was understanding the GPIO mapping between the physical pins and the sysfs GPIO numbers. All can be found in the #link("https://linux-sunxi.org/GPIO", [*sunxi driver*]) which is the driver for GPIO.

== Results
We can demonstracte that the application works in an efficient than the silly led controller given:

#table(
    columns: (1fr, 1fr),
    align: center + horizon,
    stroke: none,
    [
        #figure(
            image("test-silly.png"),
            caption:[Run silly led controller on nanopi]
        )<fig-silly>
        
    ],[
        #figure(
            image("test-epoll.png"),
            caption:[Run epoll led controller on nanopi]
        )<fig-epoll>
    ]
)

We can see the difference between @fig-silly and @fig-epoll. One is using a core at 100% and the other one not.

