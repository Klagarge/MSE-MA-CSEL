// common_ipc.h
#ifndef COMMON_IPC_H
#define COMMON_IPC_H

#define SOCKET_PATH "/tmp/regulator_daemon.sock"

/* Commands definitions */
#define CMD_SET_MODE    10
#define CMD_SET_FREQ    20
#define CMD_INC_FREQ    30
#define CMD_DEC_FREQ    40

/* Structure of the message sent through IPC */
typedef struct {
    int command;
    int value;
} ipc_msg_t;

#endif /* COMMON_IPC_H */
