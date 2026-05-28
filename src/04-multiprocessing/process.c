#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sched.h>
#include <sys/resource.h>
#include <signal.h>

const int NBR_MSG = 5;
const char * MSG[] = {
    "Hallo, hallo !\0",
    "ça geht !\0",
    "Comment vont les olives ?\0",
    "Sacré trucs tes trucs là.\0",
    "Ta où les vaches !!!!!\0"
};

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


int main(int argc, char* argv[]) {

    install_catch_signal();

    /* Setup socket for inter-process communication */
    int fd[2];
    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (err == -1) {
        perror("socketpair fail");
        exit(EXIT_FAILURE);
    }

    /* Prepare cpu set for process affinity */
    cpu_set_t set;
    CPU_ZERO(&set);
    int child_cpu = 0;
    int parent_cpu = 1;

    /* Fork a child process */
    pid_t pid = fork();

    if (pid == 0) {   /* Parent process */
        pid_t parent_pid = getpid();
        printf("Parent process: pid=%d\n", parent_pid);

        /* Setup CPU for process */
        CPU_SET(child_cpu, &set);
        int ret = sched_setaffinity(parent_pid, sizeof(set), &set);
        if (ret == -1) {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }

        /* Read messages from child */
        char buffer[100];
        for (int i = 0; i < NBR_MSG; i++) {
            read(fd[1], buffer, strlen(MSG[i]));
            printf("Message %d: %s\n", i, buffer);
            memset(buffer, 0, sizeof(buffer));
        }

    } else if (pid > 0) { /* Child process */
        pid_t child_pid = getpid();
        printf("Child process: pid=%d\n", child_pid);

        /* Setup CPU affinity for process */
        CPU_SET(parent_cpu, &set);
        int ret = sched_setaffinity(child_pid, sizeof(set), &set);
        if (ret == -1) {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }

        /* Write messages for the parent process */
        for (int i = 0; i < NBR_MSG; i++) {
            write(fd[0], MSG[i], strlen(MSG[i]));
        }

        exit(EXIT_SUCCESS);

    } else {
        /* error */
        perror("fork fail");
        exit(EXIT_FAILURE);
    }

    /* Test signal handling */
    kill(getpid(), SIGHUP);
    kill(getpid(), SIGQUIT);
    kill(getpid(), SIGTERM);
    kill(getpid(), SIGABRT);
    kill(getpid(), SIGINT);

    return EXIT_SUCCESS;
}
