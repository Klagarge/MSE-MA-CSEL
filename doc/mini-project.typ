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
#v(5em)
#infobox()[
  The repository for this labs can be found at the following address:

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
We simulate a fan controlled by the temperatur of the @cpu. To simulate this fan, we blink the status led.
The @fig:general-architecture shows the general architecture of the project.

This led and the measure of the temperature is managed by a kernel module. This module support an automatic and manual mode. In the automatic mode, the blinking frequency is automatically adjusted according to the temperature. We can switch this mode by a sysfs entry. In the manual mode, we can set the blinking frequency by writing in another sysfs entry. The sysfs also provide an entry to read the current temperature and blinking frequency.

Another part in this mini-project is to create a deamon in user-space to control manually the fan. The button are read by the deamon to increase and decrease the blinking frequency in manual mode. The deamon also display the current temperature and blinking frequency on an oled screen. The daemon can also be controller by a @ipc interface. 

Finally, a tiny CLI is implemented to control the daemon trought the @ipc interface.

= Architecture

#general-architecture

== Kernel
- everything is linked on regulator and main
=== blink
=== temperature
=== sysfs

== Daemon
=== gpio
- issue devicetree and solution
=== ipc
=== oled
=== application

== CLI

= Future work

= Conclusion
Fun, but not enough time for more over-engineering.



//-------------------------------------
// Glossary
//
#heading(numbering:none, outlined: false)[] <sec:end>
#make_glossary(gloss:gloss, title:i18n("gloss-title"))
