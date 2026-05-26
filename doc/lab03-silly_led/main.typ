#import "/doc/metadata.typ": *

= Linux System Programming

This laboratory implements a user-space application for the NanoPi NEO Plus2 that controls the blinking frequency of the status LED using three push buttons. The main goal was to replace a CPU-intensive busy loop with an event-driven design.


== Design
The application is based on multithreading: one thread handles the LED timing, while another handles button events. GPIOs are accessed through sysfs, which allows the LED and buttons to be managed as file descriptors. A key design choice was to centralize all events with a single epoll instance, so both timer events and button events can be processed efficiently. 

The timer thread use only 1 timer and set the initial time on every cycle. That allow to allocate only once the timer and avoid memory fragmentation. The button thread write write the next time to sleep on a shared variable, and the timer thread read this variable to set the next time to sleep. Since we have only one provider of this variable, we don't need to use a mutex to protect it.


All logs are done using the syslog at info level:
```c
// First, we open the syslog with a specific name and facility
// LOG_PID to include the PID (process ID) in the logs
// LOG_USER to specify the log facility (what type of programme)
openlog("CSEL Logs", LOG_PID, LOG_USER);

// Then log what you want: 
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
            image("test-silly.png", height: 10em),
            caption:[Run silly led controller on nanopi]
        )<fig-silly>
        
    ],[
        #figure(
            image("test-epoll.png", height: 10em),
            caption:[Run epoll led controller on nanopi]
        )<fig-epoll>
    ]
)

We can see the difference between @fig-silly and @fig-epoll. One is using a core at 100% and the other one not.

