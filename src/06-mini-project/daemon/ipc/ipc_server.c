// ipc_socket.c
#include "ipc_server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

/* Global variables for the IPC module */
static int server_fd = -1;
static pthread_t ipc_thread;
static struct ipc_callbacks_t current_cbs = {0};
static int is_running = 0;

/**
 * process_message() - Route the received IPC message to the correct callback.
 */
static void process_message(ipc_msg_t *msg) {
    switch (msg->command) {
        case CMD_SET_MODE:
            if (current_cbs.on_set_mode) {
                current_cbs.on_set_mode(msg->value);
            }
            break;

        case CMD_SET_FREQ:
            if (current_cbs.on_set_frequency) {
                current_cbs.on_set_frequency(msg->value);
            }
            break;

        case CMD_INC_FREQ:
            if (current_cbs.on_inc_frequency) {
                current_cbs.on_inc_frequency();
            }
            break;

        case CMD_DEC_FREQ:
            if (current_cbs.on_dec_frequency) {
                current_cbs.on_dec_frequency();
            }
            break;

        default:
            printf("[IPC] Received unknown command: %d\n", msg->command);
            break;
    }
}

/**
 * ipc_thread_func() - The main loop of the background IPC thread.
 */
static void *ipc_thread_func(void *arg) {
    ipc_msg_t msg;
    ssize_t bytes_read;

    printf("[IPC] Thread started. Listening on %s\n", SOCKET_PATH);

    while (is_running) {
        /* Block until a message arrives or the socket is closed */
        bytes_read = recvfrom(server_fd, &msg, sizeof(msg), 0, NULL, NULL);

        if (bytes_read == sizeof(ipc_msg_t)) {
            process_message(&msg);
        }
        else if (bytes_read <= 0 && is_running == 0) {
            /* Normal exit condition when shutdown() is called */
            break;
        }
        else if (bytes_read > 0) {
            printf("[IPC] Received malformed message (size %zd)\n", bytes_read);
        }
        else {
            perror("[IPC] Error receiving message");
        }
    }

    printf("[IPC] Thread stopped.\n");
    return NULL;
}

/* --- Public functions --- */

int start_ipc_server(struct ipc_callbacks_t *cbs) {
    struct sockaddr_un server_addr;

    if (is_running) {
        printf("[IPC] Server is already running.\n");
        return -1;
    }

    /* Save the provided callbacks */
    if (cbs) {
        current_cbs = *cbs;
    }

    /* Create local socket */
    server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("[IPC] Failed to create socket");
        return -1;
    }

    /* Bind socket to the file path */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH); /* Clean up existing orphaned socket file */

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[IPC] Failed to bind socket");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    /* Start the background thread */
    is_running = 1;
    if (pthread_create(&ipc_thread, NULL, ipc_thread_func, NULL) != 0) {
        perror("[IPC] Failed to create IPC thread");
        is_running = 0;
        close(server_fd);
        unlink(SOCKET_PATH);
        server_fd = -1;
        return -1;
    }

    return 0;
}

int stop_ipc_server(void) {
    if (!is_running) {
        return 0;
    }

    printf("[IPC] Stopping server...\n");

    /* Signal thread to stop */
    is_running = 0;

    /* Unblock the recvfrom() by shutting down the socket */
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        server_fd = -1;
    }

    /* Wait for thread to exit */
    pthread_join(ipc_thread, NULL);

    /* Remove the socket file */
    unlink(SOCKET_PATH);

    /* Clear callbacks */
    memset(&current_cbs, 0, sizeof(struct ipc_callbacks_t));

    return 0;
}
