// common_ipc.h
#ifndef COMMON_IPC_H
#define COMMON_IPC_H

#include <stdint.h>
#define SOCKET_PATH "/tmp/regulator_daemon.sock"

/* Commands definitions */
#define CMD_SET_MODE     10
#define CMD_SET_PERIOD   20
#define CMD_INC_PERIOD   30
#define CMD_DEC_PERIOD   40

/* Structure of the message sent through IPC */
typedef struct {
    int command;
    uint32_t value;
} ipc_msg_t;

#endif /* COMMON_IPC_H */
