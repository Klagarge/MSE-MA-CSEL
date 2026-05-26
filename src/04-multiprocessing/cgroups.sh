#!/bin/sh

mount -t tmpfs none /sys/fs/cgroup # Mount a temporary filesystem to /sys/fs/cgroup

# Memory
mkdir /sys/fs/cgroup/memory # Create a directory for the memory cgroup
mount -t cgroup -o memory cgroup /sys/fs/cgroup/memory # Mount the cgroup filesystem with memory
mkdir /sys/fs/cgroup/memory/0 # Create a subdirectory for the memory cgroup
echo $$ > /sys/fs/cgroup/memory/0/tasks # Add the current process to the memory cgroup
echo 20M > /sys/fs/cgroup/memory/0/memory.limit_in_bytes # Set the memory limit to 20 MiB

