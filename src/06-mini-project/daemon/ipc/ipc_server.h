#ifndef IPC_SERVER_H
#define IPC_SERVER_H

#include "../../common/common_ipc.h"

#include <stdint.h>

/*
 * Structure holding the callbacks for IPC commands.
 * Each function pointer will be called when the corresponding command is received.
 */
struct ipc_callbacks_t {
    void (*on_set_mode)(int mode);
    void (*on_set_period)(uint32_t period_ms);
    void (*on_inc_period)(void);
    void (*on_dec_period)(void);
} ;

/**
 * start_ipc_server() - Start the IPC background thread.
 * @cbs: Pointer to the structure containing the callback functions.
 *
 * Return: 0 on success, or a negative value on error.
 */
int start_ipc_server(struct ipc_callbacks_t *cbs);

/**
 * stop_ipc_server() - Stop the IPC thread and clean up the socket.
 *
 * Return: 0 on success.
 */
int stop_ipc_server(void);

#endif //IPC_SERVER_H
