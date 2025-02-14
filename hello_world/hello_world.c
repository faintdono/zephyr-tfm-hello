#include "psa_manifest/tfm_hello_world.h"
#include "psa/service.h"
#include <stdio.h>
#include <string.h>

#define BUFFER_LEN 32

typedef psa_status_t (*hw_service_handler_t)(psa_msg_t *);

static void handle_psa_message(psa_signal_t signal, hw_service_handler_t handler);

static psa_status_t hello_world_ipc_handler(psa_msg_t *msg);

/**
 * Secure partition entry point that continuously listens for requests.
 */
void tfm_hello_world_main(void)
{
    psa_signal_t signals = 0;

    printf("[Secure] Hello World Partition Started.\n");

    while (1) {
        signals = psa_wait(PSA_WAIT_ANY, PSA_BLOCK);
        printf("[Secure] Received signal: 0x%X\n", signals);

        if (signals & HELLO_WORLD_SERVICE_SIGNAL) {
            handle_psa_message(HELLO_WORLD_SERVICE_SIGNAL, hello_world_ipc_handler);
        } else {
            printf("[Secure] ERROR: Unexpected signal received (0x%X)\n", signals);
            psa_panic();
        }
    }
}

/**
 * Function to process Hello World service requests.
 */
static psa_status_t hello_world_ipc_handler(psa_msg_t *msg)
{
    uint8_t send_buf[BUFFER_LEN];

    strcpy((char *)send_buf, "Hello, Secure World!");

    printf("[Secure] Inside hello_world_ipc_handler()\n");
    printf("[Secure] Message Type: %d, Handle: %d\n", msg->type, msg->handle);
    printf("[Secure] msg.in_size[0]: %d, msg.out_size[0]: %d\n", msg->in_size[0], msg->out_size[0]);

    switch (msg->type) {
    case PSA_IPC_CONNECT:
        printf("[Secure] PSA_IPC_CONNECT received.\n");
        psa_reply(msg->handle, PSA_SUCCESS);
        break;

    case PSA_IPC_CALL:
        printf("[Secure] PSA_IPC_CALL received. Processing request...\n");

        if (msg->out_size[0] < strlen((char *)send_buf) + 1) {
            printf("[Secure] ERROR: Output buffer too small! Required: %d, Provided: %d\n", 
                   (int)strlen((char *)send_buf) + 1, msg->out_size[0]);
            psa_reply(msg->handle, PSA_ERROR_PROGRAMMER_ERROR);
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        psa_write(msg->handle, 0, send_buf, strlen((char *)send_buf) + 1);
        psa_reply(msg->handle, PSA_SUCCESS);
        printf("[Secure] Response sent successfully.\n");
        break;

    case PSA_IPC_DISCONNECT:
        printf("[Secure] PSA_IPC_DISCONNECT received.\n");
        psa_reply(msg->handle, PSA_SUCCESS);
        break;

    default:
        printf("[Secure] ERROR: Invalid message type received: %d\n", msg->type);
        psa_reply(msg->handle, PSA_ERROR_PROGRAMMER_ERROR);
    }

    return PSA_SUCCESS;
}



/**
 * Generic PSA request handler that retrieves and processes messages.
 */
static void handle_psa_message(psa_signal_t signal, hw_service_handler_t handler)
{
    psa_status_t status;
    psa_msg_t msg;

    status = psa_get(signal, &msg);
    if (status != PSA_SUCCESS) {
        printf("[Secure] ERROR: psa_get() failed with status: %d\n", status);
        psa_panic();
    }

    handler(&msg);
}
