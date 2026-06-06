// workspace/src/06-mini-project/cli/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

/* Relative path to your common IPC definitions */
#include "../common/common_ipc.h"

#define MODE "mode"
#define PERIOD "period"
#define INC "inc"
#define DEC "dec"


/**
 * print_usage() - Display the help menu for the CLI tool.
 */
static void print_usage(const char *prog_name) {
    printf("Temperature Regulator CLI\n");
    printf("Usage:\n");
    printf("  %s mode <0|1>      : Set mode (0 = Manual, 1 = Auto)\n", prog_name);
    printf("  %s period <ms>     : Set period in ms (only works in Manual mode)\n", prog_name);
    printf("  %s inc             : Increase frequency (only works in Manual mode)\n", prog_name);
    printf("  %s dec             : Decrease frequency (only works in Manual mode)\n", prog_name);
    printf("\nExamples:\n");
    printf("  %s mode 1          (Switch to automatic mode)\n", prog_name);
    printf("  %s period 500      (Set period to 500 ms)\n", prog_name);
}

static int process_arguments(int argc, char *argv[], ipc_msg_t *msg) {

    if (strcmp(argv[1], MODE) == 0 && argc == 3) {
        msg->command = CMD_SET_MODE;
        msg->value = atoi(argv[2]);
        if (msg->value != 0 && msg->value != 1) {
            printf("Error: Mode must be 0 (Manual) or 1 (Auto).\n");
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[1], PERIOD) == 0 && argc == 3) {
        msg->command = CMD_SET_PERIOD;
        msg->value = atoi(argv[2]);
        if (msg->value <= 0) {
            printf("Error: Frequency must be a positive integer.\n");
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(argv[1], DEC) == 0 && argc == 2) {
        msg->command = CMD_INC_PERIOD;
        msg->value = 0; /* Ignored by daemon */
    }
    else if (strcmp(argv[1], INC) == 0 && argc == 2) {
        msg->command = CMD_DEC_PERIOD;
        msg->value = 0; /* Ignored by daemon */
    }
    else {
        printf("Error: Invalid arguments.\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int client_fd;
    struct sockaddr_un server_addr;
    ipc_msg_t msg = {0};

    /* Parse command line arguments */
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (process_arguments(argc, argv, &msg) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* Create local Unix Domain Socket (Datagram) */
    client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("Error: Socket creation failed");
        return EXIT_FAILURE;
    }

    /* Prepare the destination address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    /* Send the binary structure to the Daemon */
    ssize_t sent_bytes = sendto(client_fd, &msg, sizeof(msg), 0,
                                (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (sent_bytes < 0) {
        perror("Error: Failed to send message");
        printf("Hint: Is the background daemon running and listening on '%s'?\n", SOCKET_PATH);
        close(client_fd);
        return EXIT_FAILURE;
    }

    /* 5. Cleanup */
    printf("Success: Command sent to the daemon.\n");
    close(client_fd);

    return EXIT_SUCCESS;
}
