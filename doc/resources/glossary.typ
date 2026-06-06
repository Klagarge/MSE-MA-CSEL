#import "@preview/hei-synd-report:0.1.1": *

#let entry-list = (
  (
    key: "hei",
    short: "HEI",
    long: "Haute École d'Ingénierie",
    group: "University"
  ),
  (
    key: "synd",
    short: "SYND",
    long: "Systems Engineering",
    group: "University"
  ),
  (
    key: "it",
    short: "IT",
    long: "Infotronics",
    group: "University"
  ),
  (
    key: "rust",
    short: "Rust",
    plural: "Rust programs",
    long: "Rust Programming Language",
    description: "Rust is a modern systems programming language focused on safety, speed, and concurrency. It prevents common programming errors such as null pointer dereferencing and data races at compile time, making it a preferred choice for performance-critical applications.",
    group: "Programming Language"
  ),
  (
    key: "csel",
    short: "CSEL",
    long: "Conception de Systèmes Embarqués sous Linux",
    description: "Embedded Linux Systems Design course at HES-SO, covering kernel development, driver programming, and system optimization.",
    group: "Course"
  ),
  (
    key: "cpu",
    short: "CPU",
    long: "Central Processing Unit",
    description: "The primary component of a computer that performs most of the processing inside the computer, executing instructions of computer programs.",
    group: "Hardware"
  ),
  (
    key: "l1",
    short: "L1",
    long: "Level 1 Cache",
    description: "The primary cache of a CPU, typically built directly into the processor chip, representing the fastest but smallest cache level closest to the execution units.",
    group: "Hardware"
  ),
  (
    key: "l2",
    short: "L2",
    long: "Level 2 Cache",
    description: "A secondary cache that is larger but slightly slower than the L1 cache, serving to catch cache misses from the L1 cache before querying system memory.",
    group: "Hardware"
  ),
  (
    key: "ram",
    short: "RAM",
    long: "Random-Access Memory",
    description: "A form of volatile computer memory that can be read and changed in any order, used to store working data and machine code currently in use.",
    group: "Hardware"
  ),
  (
    key: "pc",
    short: "PC",
    long: "Program Counter",
    description: "A processor register that indicates where the computer is in its program sequence, holding the address of the next instruction to be executed.",
    group: "Hardware"
  ),
  (
    key: "led",
    short: "LED",
    long: "Light Emitting Diode",
    description: "A semiconductor light source that emits light when current flows through it.",
    group: "Hardware"
  ),
  (
    key: "gpio",
    short: "GPIO",
    plural: "GPIOs",
    long: "General-Purpose Input/Output",
    description: "Uncommitted digital signal pins on an integrated circuit or electronic circuit board whose behavior can be programmed as input or output at runtime.",
    group: "Hardware"
  ),
  (
    key: "pid",
    short: "PID",
    plural: "PIDs",
    long: "Process Identifier",
    description: "A unique numerical identifier assigned by the operating system kernel to each active process, used for managing, scheduling, and tracking processes.",
    group: "Operating System"
  ),
  (
    key: "irq",
    short: "IRQ",
    plural: "IRQs",
    long: "Interrupt Request",
    description: "A signal sent to the processor that temporarily suspends the current program execution to allow an Interrupt Service Routine (ISR) to run in response to a hardware event.",
    group: "Operating System"
  ),
  (
    key: "gpu",
    short: "GPU",
    long: "Graphics Processing Unit",
    description: "A specialized electronic circuit designed to accelerate graphics rendering and parallel computing tasks.",
    group: "Hardware"
  ),
  (
    key: "cuda",
    short: "CUDA",
    long: "Compute Unified Device Architecture",
    description: "A parallel computing platform and application programming interface model created by NVIDIA.",
    group: "Programming API"
  ),
  (
    key: "openmp",
    short: "OpenMP",
    long: "Open Multi-Processing",
    description: "An application programming interface that supports multi-platform shared-memory multiprocessing programming.",
    group: "Programming API"
  ),
  (
    key: "io",
    short: "I/O",
    long: "Input/Output",
    description: "The communication between an information processing system (such as a computer) and the outside world.",
    group: "Computer Science"
  ),
  (
    key: "ip",
    short: "IP",
    plural: "IPs",
    long: "Internet Protocol",
    description: "The principal communications protocol in the Internet protocol suite for relaying datagrams across network boundaries.",
    group: "Computer Science"
  ),
  (
    key: "oom",
    short: "OOM",
    long: "Out of Memory",
    description: "A state of computer operation where no additional memory can be allocated, often leading to the invocation of an OOM killer to terminate processes.",
    group: "Operating System"
  ),
  (
    key: "sysfs",
    short: "sysfs",
    long: "System Filesystem",
    description: "A virtual pseudo-filesystem provided by the Linux kernel that exports information about hardware, device drivers, and kernel subsystems to user space.",
    group: "Operating System"
  ),
  (
    key: "syslog",
    short: "syslog",
    long: "System Logging",
    description: "A standard protocol and utility for system message logging in UNIX and Linux systems, allowing applications to log messages to files, consoles, or remote syslog daemons.",
    group: "Operating System"
  ),
  (
    key: "perf",
    short: "perf",
    long: "Performance Events for Linux",
    description: "A powerful performance supervising and analyzing tool in Linux, capable of profiling hardware performance counters, tracepoints, software performance counters, and dynamic probes.",
    group: "Operating System"
  ),
  (
    key: "epoll",
    short: "epoll",
    long: "Event Poll",
    description: "A scalable Linux I/O event notification facility designed to monitor multiple file descriptors with high efficiency.",
    group: "Operating System"
  ),
  (
    key: "cgroup",
    short: "cgroup",
    plural: "cgroups",
    long: "Control Groups",
    description: "A Linux kernel feature that limits, polices, and isolates resource usage (such as CPU, memory, and disk I/O) for groups of processes.",
    group: "Operating System"
  ),
  (
    key: "mib",
    short: "MiB",
    plural: "MiBs",
    long: "Mebibyte",
    description: "A unit of digital information equal to 1,048,576 bytes (2^20 bytes).",
    group: "Computer Science"
  ),
  (
    key: "ipc",
    short: "IPC",
    long: "Inter-Process Communication",
    description: "A set of programming interfaces that allow processes to communicate with each other and synchronize their actions.",
    group: "Operating System"
  )
)


#let make_glossary(
  gloss:true,
  title: i18n("gloss-title"),
) = {[
  #if gloss == true {[
    #pagebreak()
    #set heading(numbering: none)
    = #title <sec:glossary>
    #print-glossary(
      entry-list,
      // show all term even if they are not referenced, default to true
      show-all: false,
      // disable the back ref at the end of the descriptions
      disable-back-references: false,
    )
  ]} else{[
    #set text(size: 0pt)
    #title <sec:glossary>
    #print-glossary(
      entry-list,
      // show all term even if they are not referenced, default to true
      show-all: false,
      // disable the back ref at the end of the descriptions
      disable-back-references: false,
      shorthands: ("plural", "capitalize", "capitalize-plural", "short", "long"),
    )
  ]}
]}
