#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include <sys/wait.h>

#include <sched.h>


int fork(void);


volatile unsigned long counter = 0;
void cpu_intensive_work(const char *process_name) {
    printf("%s (PID: %d) starting CPU-intensive work on CPU %d\n", process_name, getpid(), sched_getcpu());

    while (1) {
        counter++;
        counter = (counter * counter + counter) % (counter * 1023);
    }
}

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [--single] [--dual]\n", prog);
    fprintf(stderr, "  --single    Run single process (default)\n");
    fprintf(stderr, "  --dual      Run two processes\n");
}

int main(int argc, char *argv[]) {
    int use_dual = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "--dual") == 0) {
            use_dual = 1;
        } else if (strcmp(argv[1], "--single") == 0) {
            use_dual = 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (use_dual) {
        // Dual-process mode
        pid_t pid = fork();

        if (pid == 0) {
            cpu_intensive_work("Child process");
            exit(0);
        } else if (pid > 0) {
            cpu_intensive_work("Parent process");
            wait(NULL);
            return 0;
        } else {
            perror("fork failed");
            return 1;
        }
    } else {
        // Single-process mode
        cpu_intensive_work("Process");
    }

    return 0;
}
