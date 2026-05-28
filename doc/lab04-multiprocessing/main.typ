#import "/doc/metadata.typ": *

= Multiprocessing

== Process, signals, and communication

The aim of this laboratory is to create a child process from the parent with `fork()`. Then, each processus executes the same code until they are killed. This happens the same when programming GPU with CUDA or OpenMP. The different processus are differenciated by the PID (Process ID).

The child must communicate with the parents  with a `socketpair`:
```c
/* Setup socket for inter-process communication */
    int fd[2];
    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (err == -1) {
        perror("socketpair fail");AF_UNIX
        exit(EXIT_FAILURE);
    }
```
This creates a local socket for inter-process communication. It return 2 file descriptors to read and write on the same file.

The program must handle some signal and print them:
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

There was one thing to be anticipate. If the `ctrl+c` is handled, it has to exit the process. Because the process will block the terminal. The only way to kill the process is to open in another terminal a tool like `top` or `htop`.

Finally, each processus has his own core. This setup with the `sched_setaffinity`:
```c
/* Setup CPU for process */
CPU_SET(child_cpu, &set);
int ret = sched_setaffinity(parent_pid, sizeof(set), &set);
if (ret == -1) {
    perror("sched_setaffinity");
    exit(EXIT_FAILURE);
}
```

This can be verified by executing the program and observed in the `htop` tool.

```bash
$ ./multiprocessing
Child processus: pid=273
Parent processus: pid=274
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
    caption: [Execution of the program multiprocessus]
)<multiprocessus>


The @multiprocessus shows the PID and the core of the processus and they can be compared to the output of the executable before. 
The child processus has the PID 273 and the core 0. The parent processus has th PID 274 and the core 1.

== CGroups memory

The goal of this part is to understand how to use cgroups to limit the resources of a process. We will initially focus on memory, but cgroups can also be used to limit CPU, I/O, and other ressources.

For limit the memory usage of a process, we cans use the `memory` subsystem of cgroups. We use cgroup v1 with our Nanopi.

We must first mount a temporary filesystem for cgroups:
```bash
|> mount -t tmpfs none /sys/fs/cgroup
```

We can the create a directory for the memory cgroup, mount the cgroup filesystem with memory, and create a subdirectory for our cgroup:

```bash
# Create a directory for the memory cgroup
|> mkdir /sys/fs/cgroup/memory

# Mount the cgroup filesystem with memory
|> mount -t cgroup -o memory cgroup /sys/fs/cgroup/memory

# Create a subdirectory for the memory cgroup
|> mkdir /sys/fs/cgroup/memory/0
```

We can then add the current process to this memory cgroups and set a memory limit of 20 MiB:

```bash
# Add the current process to the memory cgroup
|> echo $$ > /sys/fs/cgroup/memory/0/tasks

# Set the memory limit to 20 MiB
|> echo 20M > /sys/fs/cgroup/memory/0/memory.limit_in_bytes
```

We can then run our test program that allocates memory in a loop and see what happens when we exceed the memory limit.

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

We can use the `cgroups.sh` script in `04-multiprocessing` to set up the cgroup and run the test program, but we need to run with the actual context, so we need to execute the script with `.`:

```bash
|> just cgroups # Build the test program
|> . cgroups.sh # Run the script in the current shell
|> ./cgroups # Run the test program that allocates memory in a loop
```

=== What is the behavior of the command `echo $$ > ...` on cgroups?

The `$$` represent the current process ID (PID). When we execute the command `echo $$ > /sys/fs/cgroup/memory/0/tasks`, we are writing the PID of the current process into the `tasks` file of the specified cgroup. This action effectively assigns the process to that cgroup, meaning that it will now be subject to the resource limits and policies defined for that cgroup.


=== What is the behavior of the memory subsystem when the memory quota is exhausted? Can we modify it? If yes, how?

For this nanopi, we use cgroup v1, so the relevant file is `memory.limit_in_bytes`. When a process within a cgroup exceeds the memory limit defined by `memory.limit_in_bytes`, the Linux kernel will attempt to reclaim memory. If it cannot reclaim enough memory, it will invoke the Out Of Memory (OOM) killer to terminate processes within that cgroup to free up memory.

It's possible to modify this behavior in several ways:

+ Use "Soft Limits" (Specific to cgroup v1)
  In addition to a hard limit (`memory.limit_in_bytes`), you can set a soft limit (`memory.soft_limit_in_bytes`).
  *Behavior:* The kernel will not kill the process if the soft limit is exceeded, unless the entire system is low on global memory. If global memory is low, the kernel will start reclaiming memory from groups that exceed their soft limit.

+ Adjust the OOM Killer Priority Score
  If we specify an OOM score adjustement for the process. By modifying the file `/proc/[PID]/oom_score_adj` with the value `-1000`, we can make the process almost "immune" to the OOM Killer.

=== How to watch the memory usage?

We can monitor the memory usage of a cgroup by reading it directly from the file in the specific cgroups:

```bash
# Current memory usage in bytes
|> cat /sys/fs/cgroup/memory/0/memory.usage_in_bytes
212992

# Maximum memory usage in bytes
|> cat /sys/fs/cgroup/memory/0/memory.max_usage_in_bytes
20971520
```

== CGroups CPU
To check this part, we need a tiny program that consumes CPU with at least two process.
The following program creates a child process that performs CPU intensive work, while the parent process also performs CPU intensive work. We can then use cgroups to limit the CPU usage of one of the processes and observe the effect.
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

Based on previous exercice, we should already have mounted the cgroup filesystem.
```bash
|>  mount -t tmpfs none /sys/fs/cgroup
```

We can then create and mount the cgroup filesystem for the `cpuset` subsystem
```bash
# Create a directory for the cpuset cgroup
|> mkdir /sys/fs/cgroup/cpuset

# Mount the cgroup filesystem with cpuset
|> mount -t cgroup -o cpu,cpuset cpuset /sys/fs/cgroup/cpuset
```

Now we had the prerequirements, we can create 2 groupes. One for each of our running programme. With the following command, we attribute on ore more CPU to each group (`cpuset.cpus`). I'm not sure about the `cpuset.mems` file, but it seems to be related to memory nodes. It's definetly a topic that should be explored more in depth, but for now, we set to `0` as specified in the lab instructions.

```bash
# Create and allocate CPU for programme "low"
|> mkdir /sys/fs/cgroup/cpuset/low
|> echo 1 > /sys/fs/cgroup/cpuset/low/cpuset.cpus
|> echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.mems

# Create and allocate CPU for programme "high"
|> mkdir /sys/fs/cgroup/cpuset/high
|> echo 2,3 > /sys/fs/cgroup/cpuset/high/cpuset.cpus
|> echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.mems
```

We can then open 2 shells and run the test program in each of them, while adding the programme to the corresponding cgroup:
```bash
# In the first shell, add it on the "low" cgroup and run the test program
|> . ./max-cpu.sh low

# In the second shell, add it on the "high" cgroup and run the test program
|> . ./max-cpu.sh high
```

We see on @max-cpu that as expected, both process in program _low_ is limited to CPU 1, while the programm _high_ is using CPU 2 and 3, one for each process.

#figure(
    image("max-cpu.png"),
    caption: [CPU usage of the two programmes with dedicated resources]
)<max-cpu>

To share resources at 75% and 25%, we can use the `cpu.shares` file in the `cpu` cgroup. We attribute a value 3 time high for the _high_ group than for the _low_ group.

```bash
|> echo 75 > /sys/fs/cgroup/cpu/high/cpu.shares
|> echo 25 > /sys/fs/cgroup/cpu/low/cpu.shares
```

Then running the test program in each shell, we see on @shared-cpu that the _high_ process is limited to 75% of the CPU, while the _low_ process is limited to 25%.
```bash
# In the first shell, add it on the "low" cgroup and run the test program
|> . ./shared-cpu.sh low

# In the second shell, add it on the "high" cgroup and run the test program
|> . ./shared-cpu.sh high
```


#figure(
    image("shared-cpu.png"),
    caption: [CPU usage of the two programmes with shared resources]
)<shared-cpu>