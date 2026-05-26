#import "/doc/metadata.typ": *

= Multiprocessing

== Process, signals, and communication

// TODO Yann

== CGroups

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
