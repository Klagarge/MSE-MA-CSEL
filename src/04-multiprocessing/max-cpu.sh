#!/bin/sh

usage() {
    echo "Usage: $0 {init|high|low}"
    echo "  init  - Initialize cgroup filesystem and cpuset groups"
    echo "  high  - Run ./max-cpu --dual in the 'high' cgroup (CPUs 2,3)"
    echo "  low   - Run ./max-cpu --dual in the 'low' cgroup (CPU 1)"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

case "$1" in
    init)
        echo "Initializing cgroup filesystem..."

        # Mount tmpfs for cgroup
        mount -t tmpfs none /sys/fs/cgroup 2>/dev/null

        # Create and mount cpuset cgroup
        mkdir -p /sys/fs/cgroup/cpuset
        mount -t cgroup -o cpu,cpuset cpuset /sys/fs/cgroup/cpuset 2>/dev/null

        # Create "low" cgroup and allocate CPU 1
        mkdir -p /sys/fs/cgroup/cpuset/low
        echo 1 > /sys/fs/cgroup/cpuset/low/cpuset.cpus
        echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.mems

        # Create "high" cgroup and allocate CPUs 2,3
        mkdir -p /sys/fs/cgroup/cpuset/high
        echo 2,3 > /sys/fs/cgroup/cpuset/high/cpuset.cpus
        echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.mems

        echo "Cgroup initialization complete."
        echo "  - low group:  CPU 1"
        echo "  - high group: CPUs 2,3"
        ;;

    high)
        echo "Running ./max-cpu --dual in 'high' cgroup (CPUs 2,3)..."
        echo $$ > /sys/fs/cgroup/cpuset/high/tasks
        ./max-cpu --dual
        ;;

    low)
        echo "Running ./max-cpu --dual in 'low' cgroup (CPU 1)..."
        echo $$ > /sys/fs/cgroup/cpuset/low/tasks
        ./max-cpu --dual
        ;;

    *)
        echo "Unknown command: $1"
        usage
        ;;
esac
