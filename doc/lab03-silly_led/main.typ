#import "/doc/metadata.typ": *

= Linux System Programming

This laboratory implements a user-space application for the NanoPi NEO Plus2 that controls the blinking frequency of the status #gls("led", long: false) using three push buttons. The main goal was to replace a #gls("cpu", long: false)-intensive busy loop with an event-driven design.


== Design
The application is based on multithreading: one thread handles the #gls("led", long: false) timing, while another handles button events. #gls("gpio", long: false) are accessed through #gls("sysfs", long: false), which allows the #gls("led", long: false) and buttons to be managed as file descriptors. A key design choice was to centralize all events with a single #gls("epoll", long: false) instance, so both timer events and button events can be processed efficiently.

The timer thread uses only one timer and sets the initial time on every cycle. This allows us to allocate resources only once for the timer and avoid memory fragmentation. The button thread writes the next sleep duration to a shared variable, which the timer thread reads to set its next sleep interval. Since there is only one writer for this variable, we do not need a mutex to protect it.


All logs are written to #gls("syslog", long: false) at the INFO level:
```c
// First, we open the syslog with a specific name and facility
// LOG_PID to include the PID (process ID) in the logs
// LOG_USER to specify the log facility (what type of program)
openlog("CSEL Logs", LOG_PID, LOG_USER);

// Then log what you want:
syslog(LOG_INFO, "Start logging silly led-controller"); // INFO level
```

== Difficulties
The most difficult part was understanding the #gls("gpio", long: false) mapping between the physical pins and the #gls("sysfs", long: false) #gls("gpio", long: false) numbers. This mapping can be found in the #link("https://linux-sunxi.org/GPIO", [*sunxi driver*]) documentation, which describes the driver for the #gls("gpio", long: false) controller.

== Results
We can demonstrate that the application works more efficiently than the provided silly #gls("led", long: false) controller:

#table(
    columns: (1fr, 1fr),
    align: center + horizon,
    stroke: none,
    [
        #figure(
            image("test-silly.png", height: 10em),
            caption:[Running the silly #gls("led", long: false) controller on the NanoPi]
        )<fig-silly>

    ],[
        #figure(
            image("test-epoll.png", height: 10em),
            caption:[Running the #gls("epoll", long: false)-based #gls("led", long: false) controller on the NanoPi]
        )<fig-epoll>
    ]
)

We can see the difference between @fig-silly and @fig-epoll. One utilizes 100% of a CPU core, whereas the other does not.
