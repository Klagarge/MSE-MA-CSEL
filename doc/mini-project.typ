// #import "@preview/hei-synd-report:0.1.1": *
#import "@preview/hei-synd-thesis:0.2.3": *
#import "/doc/metadata.typ": *
#import "/doc/resources/glossary.typ": *
#show:make-glossary
#register-glossary(entry-list)

#import "@preview/fractusist:0.1.1":*

//-------------------------------------
// Template config
//

#{
  doc.title = "Mini-Project"
  doc.subtitle = [
    Mini-Project
  ]
  doc.logos.tp_main = dragon-curve(
    12,
    step-size: 10,
    stroke-style: stroke(
      //paint: gradient.linear(..color.map.rocket, angle: 135deg),
      paint: gradient.radial(..color.map.rocket),
      thickness: 3pt, join: "round"),
    height: 10cm,
  )
  doc.version = "v0.1.0"
}

#show: report.with(
  option: option,
  doc: doc,
  date: date,
  tableof: tableof,
)
#import "@preview/codly:1.3.0": *
#import "@preview/codly-languages:0.1.8": *
#show: codly-init.with()

#codly(languages: codly-languages)

#v(5em)
#infobox()[
  The repository for these lab can be found at the following address:

  #align(center)[https://github.com/Klagarge/MSE-MA-CSEL]
]
#pagebreak()

//-------------------------------------
// Content
//
#let general-architecture = [
  #figure(
    image("mini-project/deployement.png", width: 100%),
    caption: "General architecture"
  ) <fig:general-architecture>
]


= Introduction

The purpose of this mini-project is to train different concept we saw during the semester.
We simulate a fan controlled by the temperature of the @cpu. To simulate this fan, we blink the status @led.
The @fig:general-architecture shows the general architecture of the project.

This @led and the measure of the temperature is managed by a kernel module. This module support an automatic and manual mode. In the automatic mode, the blinking frequency is automatically adjusted according to the temperature. We can switch this mode by a @sysfs entry. In the manual mode, we can set the blinking frequency by writing in another @sysfs entry. The @sysfs also provide an entry to read the current temperature and blinking frequency.

Another part in this mini-project is to create a daemon in user-space to control manually the fan. The buttons are read by the daemon to increase and decrease the blinking frequency in manual mode. The daemon also displays the current temperature and blinking frequency on an @oled screen. The daemon can also be controlled by an @ipc interface.

Finally, a tiny @cli is implemented to control the daemon through the @ipc interface.

#general-architecture

== Architecture

In our architecture,  we manage to separate with callback our functionalities. Then, we use threads for multiprocessing which involve to implement some atomic operations, signals and mutex. We add socket pair and @sysfs for communication. Finally, we get some information through registers.

== Kernel
The kernel part is separated in three main parts: the blink, the temperature, and the @sysfs. All this part are initialized in the main but handle in a regulator that build the logic of the auto/man mode. In auto mode, the regulator sets the frequency according to the temperature. The regulator also handles the @sysfs for setting the mode and the frequency in case of the manual mode.

=== blink
The `blink.c` and `blink.h` files implement the part that control the status @led. It's a kernel module, so we have an init and an exit function. The init function create a kernel thread that blink to a specific frequency. The exit function stop this thread. The period is stored in a global `atomic_t` variable, so it can be safely set with the `adjust_period` function.
=== temperature

The read of temperature is done through the register. It implements the function to calculate the temperature from the register. It changes the formula when the temperature is over 70 °C, as specified in the datasheet.

=== #gls("sysfs", long: false)

It uses some callbacks for every action in the module:
- read temperature
- set and get mode
- set period
- get period

We separate the setter and the getter of the period to avoid some issue. Because if we set a wrong value or in automatic mode, the value would be wrong for getting it. In the way we did it, the read value will be the current.

== Daemon

The daemon has the core in `app`. It handles the `sysfs` functions needed by the different features. It provides them for the @oled screen, buttons, @led:pl and @ipc server.

=== #gls("gpio", long: false)
We develop the @gpio part as near as possible with a pseudo class for the @led and a pseudo class for the button. The @led class is quite simple and help to have a good understanding of this principle. As shown in @fig:led-class-header, we create a structure for the @led. A `LED_init` function is used to create a @led object by returning a pointer to this structure. Function to this class start with the same prefix `LED_` and take a pointer to the structure as parameter.

#figure(
  [```c
    typedef enum {
        LED_STATUS,
        LED_POWER,
    } LED_type; // enum to choose which led we want initialize

    typedef struct {
        int gpio;
    } LED; // Type for our led class

    LED* LED_init(LED_type type); // Create new LED object
    void LED_on(LED* led);
    void LED_off(LED* led);
    void LED_toggle(LED* led);
  ```],
  caption: "Led class header"
) <fig:led-class-header>

We develop the button in the same way, with class spirit. But a button has no function to control it, but only a callback that need to be set as shown in @fig:button-class-header. So this pseudo class abstract the complexity of a button, and we provide a simple @api with a nice callback system. Behind the scene, we have a thread that looks the button file with an `@epoll` and call the callback when the button is pushed. The first button to be initialized create this static thread. All new buttons are added on the event list of this thread.

#figure(
  [```c
    typedef void (*BTN_callback)();

    BTN* BTN_init(BTN_type type);
    void BTN_set_callback(BTN* btn, BTN_callback callback);
  ```],
  caption: "Button class header"
) <fig:button-class-header>

=== #gls("ipc", long: false)

The @ipc provides a server to handle messages from other processes with a socket pair. All is defined in a common file: `src/06-mini-project/common/common_ipc.h`. This file implements the action and the format of the message through the socket.

=== #gls("oled", long: false)
The @oled part has nothing special, we basically use the provided example. But we had to modify the devicetree to add the @i2c that control the screen. It was the first time we had to modify the buildroot part. We forgot a bit how it's absolutely not enough to modify in `/config/board/.../nanopi-neo-plus2.dts`. In the `get-buildroot.sh` script, there is a rsync command that was done only at the full beginning of the semester when we initialize everything. To effectively modify the devicetree, we had to copy our modification, then rebuild (it's short because most parts are already built):

```bash
rsync -a /workspace/config/board/ /buildroot/board/
rsync -a /workspace/config/configs/ /buildroot/configs/
cd /buildroot
make linux-rebuild
make uboot-rebuild
make
```

Then, if we boot with @tftp, we can simply reboot. Otherwise, we have to reflash the sd card with the new image.

=== application

This part is the core of the daemon and provides @api for the @oled screen, the buttons, and the @ipc to set and get values from the module. It uses @sysfs technology to communicate with the kernel.

It implements some specific action like increase and decrease the period of the @led. It provides too specifics functions for the buttons because it has to signal with the power @led when it is pushed. We called that an animation.

This animation is managed by a signal and a condition. The function increase and decrease for the buttons increment a counter and send a signal to the animation thread. It handles it and makes the animation until the counter reach 0. Then it waits with the `pthread_cond_wait`.



== #gls("cli", long: false)

The @cli is connected to the daemon through the socketpair define in the `common` as the @ipc. It uses the same struct and actions for changing mode or set, increase, decrease a period.

It is installed in the `/usr/bin` by the `justfile`. It allows using it from everywhere in the terminal. It can be used as a tool.

= Future work
#let link-github-project = "https://github.com/users/Klagarge/projects/3/views/1?filterQuery=is%3Aopen"
C.f.: #link(link-github-project)[GitHub project] #footnote(link-github-project)


= Conclusion
Fun, but not enough time for more over-engineering.



//-------------------------------------
// Glossary
//
#heading(numbering:none, outlined: false)[] <sec:end>
#make_glossary(gloss:gloss, title:i18n("gloss-title"))
