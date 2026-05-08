#include <stdio.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <syslog.h>
#include <string.h>

#include <signal.h>


const char * MSG = "Nique ta mère !!\0";

static void catch_signal(int signal) {

    switch (signal) {
        case SIGHUP:
            printf("SIGHUP received\n");
            break;
        case SIGINT:
            printf("SIGINT received\n");
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
        /* Parent processus */
        printf("Parent processus: pid=%d\n", pid);



        while(1) {
            write(fd[0], MSG, strlen(MSG));
            sleep(1);
        }


    } else if (pid > 0) {
        /* Child processus */
        printf("Child processus: pid=%d\n", pid);
        char buffer[strlen(MSG)];

        while(1) {
            read(fd[1], buffer, strlen(MSG));
            printf("%s\n", buffer);
            sleep(1);
        }

    } else {
        /* error */
        perror("fork fail");
        exit(1);
    }


    return 0;
}
