#import "/doc/metadata.typ": *

= Multiprocessing

== Process, signals, and communication

The aim of this laboratory is to create a child process from a parent process using `fork()`. Both processes then execute the same code until they are terminated. This is similar to parallel programming with #gls("gpu", long: false) using #gls("cuda", long: false) or #gls("openmp", long: false). The processes are differentiated by their #gls("pid", long: false).

The child process must communicate with the parent process using a `socketpair`:
```c
/* Setup socket for inter-process communication */
    int fd[2];
    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (err == -1) {
        perror("socketpair fail");
        exit(EXIT_FAILURE);
    }
```
This creates a local UNIX socket pair for inter-process communication. It returns two file descriptors for bidirectional communication.

The program must handle several signals and print their names when received:
```c
static void catch_signal(int signal) {

    switch (signal) {
        case SIGHUP:
            printf("SIGHUP received\n");
            break;
        case SIGINT:
            printf("SIGINT received\n");
            exit(EXIT_SUCCESS); // to avoid to be blocked and kill it with ctrl+c
            break;
        case SIGQUIT:
            printf("SIGQUIT received\n");
            break;
        case SIGTERM:
            printf("SIGTERM received\n");
            break;
        case SIGABRT:
            printf("SIGABRT received\n");
            break;
    }


}

static void install_catch_signal()
{
    struct sigaction act = {
        .sa_handler = catch_signal,
    };
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, 0);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGABRT, &act, 0);
}
```

One important design consideration to anticipate was signal handling behaviour. If `Ctrl+C` (SIGINT) is caught, but the handler does not terminate the process, the application would continue to run and block the terminal. In that case, the only way to kill the process would be to open another terminal and use a tool like `top` or `htop`.

Finally, each process is pinned to its own CPU core. This is configured using `sched_setaffinity`:
```c
/* Setup CPU affinity for process */
CPU_SET(child_cpu, &set);
int ret = sched_setaffinity(parent_pid, sizeof(set), &set);
if (ret == -1) {
    perror("sched_setaffinity");
    exit(EXIT_FAILURE);
}
```

This can be verified by executing the program and observing CPU usage in `htop`.

```bash
$ ./multiprocessing
Child process: pid=273
Parent process: pid=274
Message 0: Hallo, hallo !
Message 1: ça geht !
Message 2: Comment vont les olives ?
Message 3: Sacré trucs tes trucs là.
Message 4: Ta où les vaches !!!!!
SIGHUP received
SIGQUIT received
SIGTERM received
SIGABRT received
SIGINT received

```
#figure(
    image("control_cpu_process_ex_1.png"),
    caption: [Execution of the multiprocessing program]
)<multiprocessus>


The @multiprocessus shows the #gls("pid", long: false) and the assigned CPU core for each process, which can be compared with the console output shown above.
The child process has PID 273 and runs on core 0, whereas the parent process has PID 274 and runs on core 1.

== #glspl("cgroup", long: false) memory

The goal of this part is to understand how to use #glspl("cgroup", long: false) to limit the resources of a process. We will initially focus on memory, but #glspl("cgroup", long: false) can also be used to limit #gls("cpu", long: false), #gls("io", long: false), and other resources.

To limit the memory usage of a process, we can use the `memory` subsystem of #glspl("cgroup", long: false). On this NanoPi, we use #glspl("cgroup", long: false) v1.

We must first mount a temporary file system for #glspl("cgroup", long: false):
```bash
|> mount -t tmpfs none /sys/fs/cgroup
```

We can then create a directory for the memory subsystem, mount the corresponding #glspl("cgroup", long: false) file system, and create a subdirectory for our specific group:

```bash
# Create a directory for the memory cgroup
|> mkdir /sys/fs/cgroup/memory

# Mount the cgroup filesystem with memory
|> mount -t cgroup -o memory cgroup /sys/fs/cgroup/memory

# Create a subdirectory for the memory cgroup
|> mkdir /sys/fs/cgroup/memory/0
```

We can then add the current process to this memory #gls("cgroup", long: false) and set a memory limit of 20 #gls("mib", long: false):

```bash
# Add the current process to the memory cgroup
|> echo $$ > /sys/fs/cgroup/memory/0/tasks

# Set the memory limit to 20 MiB
|> echo 20M > /sys/fs/cgroup/memory/0/memory.limit_in_bytes
```

We can then run our test program that allocates memory in a loop to see what happens when we exceed the memory limit.

```c
for (i = 0; i < NUM_BLOCKS; i++) {

    // Allocate a block of memory
    blocks[i] = malloc(BLOCK_SIZE);

    // [...]
    // check if failed and error, clean and exit

    // Touch the memory to ensure it's actually allocated
    memset(blocks[i], 0, BLOCK_SIZE);
}
```

We can use the `cgroups.sh` script in `04-multiprocessing` to set up #glspl("cgroup", long: false) and run the test program. However, to execute the script in the context of our current shell, we must source it using the `.` command:

```bash
|> just cgroups # Build the test program
|> . cgroups.sh # Run the script in the current shell
|> ./cgroups # Run the test program that allocates memory in a loop
```

=== What is the behaviour of the command `echo $$ > ...` on #glspl("cgroup", long: false)?

The `$$` shell variable represents the #gls("pid", long: false) of the current shell. When we execute the command `echo $$ > /sys/fs/cgroup/memory/0/tasks`, we write the PID of the current shell process into the `tasks` file of the specified cgroup. This action assigns the process to that control group, meaning that any program run from this shell will inherit the resource limits and policies defined for that cgroup.


=== What is the behaviour of the memory subsystem when the memory quota is exhausted? Can we modify it? If yes, how?

On this NanoPi, we use #glspl("cgroup", long: false) v1, so the resource configuration is done via the `memory.limit_in_bytes` file. When a process within a #gls("cgroup", long: false) exceeds the memory limit defined by this file, the Linux kernel will attempt to reclaim memory. If it cannot reclaim sufficient memory, it will invoke the #gls("oom", long: false) killer to terminate processes within that #gls("cgroup", long: false) to free up memory.

It is possible to modify this behaviour in several ways:

+ *Use "Soft Limits" (specific to #glspl("cgroup", long: false) v1):*
  In addition to a hard limit (`memory.limit_in_bytes`), a soft limit can be set via `memory.soft_limit_in_bytes`.
  *Behaviour:* The kernel does not kill the process when the soft limit is exceeded, unless the entire system runs low on memory. If global memory is low, the kernel begins reclaiming memory from cgroups that exceed their soft limits.

+ *Adjust the #gls("oom", long: false) Killer priority score:*
  We can specify an #gls("oom", long: false) score adjustment for the process. By modifying the `/proc/[PID]/oom_score_adj` file to the value `-1000`, the process becomes virtually immune to the #gls("oom", long: false) killer.

=== How to watch the memory usage?

We can monitor the memory usage of a control group by reading directly from its configuration files:

```bash
# Current memory usage in bytes
|> cat /sys/fs/cgroup/memory/0/memory.usage_in_bytes
212992

# Maximum memory usage in bytes
|> cat /sys/fs/cgroup/memory/0/memory.max_usage_in_bytes
20971520
```

== #glspl("cgroup", long: false) CPU
To check this part, we need a tiny program that consumes #gls("cpu", long: false) with at least two processes.
The following program creates a child process that performs #gls("cpu", long: false)-intensive work, while the parent process also performs #gls("cpu", long: false)-intensive work. We can then use #glspl("cgroup", long: false) to limit the #gls("cpu", long: false) usage of one of the processes and observe the effect.
```c
int main() {
  pid_t pid = fork();

  if (pid == 0) {
    cpu_intensive_work("Child process");
    exit(0);
  } else {
    cpu_intensive_work("Parent process");
    wait(NULL);
    return 0;
  }
}
```

Based on the previous exercise, we should already have mounted the #glspl("cgroup", long: false) file system.
```bash
|>  mount -t tmpfs none /sys/fs/cgroup
```

We can then create and mount the #glspl("cgroup", long: false) file system for the `cpuset` subsystem:
```bash
# Create a directory for the cpuset cgroup
|> mkdir /sys/fs/cgroup/cpuset

# Mount the cgroup filesystem with cpuset
|> mount -t cgroup -o cpu,cpuset cpuset /sys/fs/cgroup/cpuset
```

With these prerequisites met, we can create two groups, one for each instance of our running program. Using the commands below, we assign one or more #gls("cpu", long: false) cores to each group via `cpuset.cpus`. I'm not sure about the `cpuset.mems` file, but it seems to be related to memory nodes. It's definitely a topic that should be explored more in depth, but for now, we set to `0` as specified in the lab instructions:

```bash
# Create and allocate CPU for program "low"
|> mkdir /sys/fs/cgroup/cpuset/low
|> echo 1 > /sys/fs/cgroup/cpuset/low/cpuset.cpus
|> echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.mems

# Create and allocate CPU for program "high"
|> mkdir /sys/fs/cgroup/cpuset/high
|> echo 2,3 > /sys/fs/cgroup/cpuset/high/cpuset.cpus
|> echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.mems
```

We can then open two shells and run the test program in each of them, while adding each program to its corresponding control group:
```bash
# In the first shell, add it to the "low" cgroup and run the test program
|> . ./max-cpu.sh low

# In the second shell, add it to the "high" cgroup and run the test program
|> . ./max-cpu.sh high
```

As shown in @max-cpu, as expected, both processes in the "low" program are limited to #gls("cpu", long: false) core 1, while the "high" program uses #gls("cpu", long: false) cores 2 and 3 (one for each process).

#figure(
    image("max-cpu.png"),
    caption: [CPU usage of the two programs with dedicated resources]
)<max-cpu>

To share resources at 75% and 25%, we can use the `cpu.shares` file in the `cpu` cgroup. We assign a share value to the "high" group that is three times higher than that of the "low" group:

```bash
|> echo 75 > /sys/fs/cgroup/cpu/high/cpu.shares
|> echo 25 > /sys/fs/cgroup/cpu/low/cpu.shares
```

After running the test program in each shell, we can observe in @shared-cpu that the processes in the "high" #gls("cgroup", long: false) are allocated 75% of the CPU capacity, while those in the "low" #gls("cgroup", long: false) receive 25%:
```bash
# In the first shell, add it to the "low" cgroup and run the test program
|> . ./shared-cpu.sh low

# In the second shell, add it to the "high" cgroup and run the test program
|> . ./shared-cpu.sh high
```


#figure(
    image("shared-cpu.png"),
    caption: [CPU usage of the two programs with shared resources]
)<shared-cpu>
