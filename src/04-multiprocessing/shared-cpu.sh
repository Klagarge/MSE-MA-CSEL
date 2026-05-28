#!/bin/sh

usage() {
    echo "Usage: $0 {init|high|low}"
    echo "  ./shared-cpu.sh init   - Initialize cgroup filesystem with cpu.shares groups"
    echo "  . ./shared-cpu.sh high - Run ./max-cpu --single in 'high' cgroup (75% CPU share on CPU 0)"
    echo "  . ./shared-cpu.sh low  - Run ./max-cpu --single in 'low' cgroup (25% CPU share on CPU 0)"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

# Helper to check if a directory is already mounted
is_mounted() {
    mount | grep -q " $1 "
}

case "$1" in
    init)
        echo "Initializing cgroup filesystem for CPU shares..."

        # Mount tmpfs for cgroup if not already mounted
        if ! is_mounted /sys/fs/cgroup; then
            echo "Mounting /sys/fs/cgroup..."
            mount -t tmpfs none /sys/fs/cgroup || {
                echo "ERROR: Failed to mount /sys/fs/cgroup"
                exit 1
            }
        else
            echo "/sys/fs/cgroup is already mounted."
        fi

        # Create and mount cpuset cgroup if not already mounted
        mkdir -p /sys/fs/cgroup/cpuset
        if ! is_mounted /sys/fs/cgroup/cpuset; then
            echo "Mounting cpuset/cpu cgroup..."
            mount -t cgroup -o cpu,cpuset cpuset /sys/fs/cgroup/cpuset || {
                echo "ERROR: Failed to mount cpuset cgroup! Perhaps 'cpu' and 'cpuset' are already mounted separately elsewhere?"
                exit 1
            }
        else
            echo "/sys/fs/cgroup/cpuset is already mounted."
        fi

        # Create "low" cgroup with 25% share
        echo "Configuring 'low' cgroup..."
        mkdir -p /sys/fs/cgroup/cpuset/low
        echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.cpus || echo "WARNING: Failed to write cpuset.cpus for low cgroup"
        echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.mems || echo "WARNING: Failed to write cpuset.mems for low cgroup"
        echo 25 > /sys/fs/cgroup/cpuset/low/cpu.shares || echo "WARNING: Failed to write cpu.shares for low cgroup"

        # Create "high" cgroup with 75% share
        echo "Configuring 'high' cgroup..."
        mkdir -p /sys/fs/cgroup/cpuset/high
        echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.cpus || echo "WARNING: Failed to write cpuset.cpus for high cgroup"
        echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.mems || echo "WARNING: Failed to write cpuset.mems for high cgroup"
        echo 75 > /sys/fs/cgroup/cpuset/high/cpu.shares || echo "WARNING: Failed to write cpu.shares for high cgroup"

        echo "Cgroup initialization complete."
        echo "  - low group:  25 shares (25% CPU on CPU 0)"
        echo "  - high group: 75 shares (75% CPU on CPU 0)"
        ;;

    high)
        echo "Running ./max-cpu --single in 'high' cgroup (75% CPU share)..."
        if [ ! -f /sys/fs/cgroup/cpuset/high/tasks ]; then
            echo "ERROR: Cgroup is not initialized or mounted! Run './shared-cpu.sh init' first."
            exit 1
        fi
        echo $$ > /sys/fs/cgroup/cpuset/high/tasks || {
            echo "ERROR: Failed to add shell process ($$) to high cgroup!"
            exit 1
        }
        ./max-cpu --single
        ;;

    low)
        echo "Running ./max-cpu --single in 'low' cgroup (25% CPU share)..."
        if [ ! -f /sys/fs/cgroup/cpuset/low/tasks ]; then
            echo "ERROR: Cgroup is not initialized or mounted! Run './shared-cpu.sh init' first."
            exit 1
        fi
        echo $$ > /sys/fs/cgroup/cpuset/low/tasks || {
            echo "ERROR: Failed to add shell process ($$) to low cgroup!"
            exit 1
        }
        ./max-cpu --single
        ;;

    *)
        echo "Unknown command: $1"
        usage
        ;;
esac
