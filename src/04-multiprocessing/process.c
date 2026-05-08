#include <stdio.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <syslog.h>
#include <string.h>

#define __USE_GNU // not the define from the professor !!!!!!!!!!!!!!!!!!!!!
#include <sched.h>
#include <sys/resource.h>

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
            exit(EXIT_SUCCESS);
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
        exit(1);
    }

    /* Fork a child process */
    pid_t pid = fork();
    if (pid == 0) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        int ret = sched_setaffinity(pid, sizeof(set), &set);
        if (ret == -1) {
            perror("sched_setaffinity");
            exit(1);
        }

        /* Parent processus */
        printf("Parent processus: pid=%d\n", pid);
        pid_t child_pid = getpid();
        char buffer[100];

        for (int i = 0; i < 7; i++) {
            read(fd[1], buffer, strlen(MSG[i]));
            printf("%s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }

        int status = 0;
        int waited_pid = waitpid(pid, &status, 0);

        if (waited_pid == -1) {
            perror("waitpid");
        } else {
            if (WIFEXITED(status)) {
                printf("Child exited with status %d\n",
                       WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child killed by signal %d\n",
                       WTERMSIG(status));
            }
        }

    } else if (pid > 0) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(1, &set);
        int ret = sched_setaffinity(pid, sizeof(set), &set);
        if (ret == -1) {
            perror("sched_setaffinity");
            exit(1);
        }
        /* Child processus */
        printf("Child processus: pid=%d\n", pid);

        for (int i = 0; i < NBR_MSG; i++) {
            if(i==2) sleep(2);
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

    return 0;
}
