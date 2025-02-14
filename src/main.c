#include <stdio.h>
#include "psa/client.h"
#include "psa_manifest/sid.h"

#define BUFFER_LEN 32

int main(void) {
    psa_handle_t handle;
    char response[BUFFER_LEN];
    psa_invec in_vec[] = {NULL, 0};
    psa_outvec out_vec[] = {response, sizeof(response)};

    printf("[NS] Requesting Hello World service...\n");

    handle = psa_connect(HELLO_WORLD_SERVICE_SID, 1);
    if (handle <= 0) {
        printf("[NS] ERROR: Failed to connect to Secure Partition (handle: %d)\n", handle);
        return -1;
    }
    
    printf("[NS] Connected to Secure Partition. Handle: %d\n", handle);
    
    // Debug buffer sizes before psa_call()
    printf("[NS] Sending psa_call() with output buffer size: %d\n", (int)sizeof(response));
    
    psa_status_t status = psa_call(handle, PSA_IPC_CALL, in_vec, IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));
    
    printf("[NS] Secure call returned status: %d\n", status);
    
    if (status == PSA_SUCCESS) {
        printf("[NS] Response received: %s\n", response);
    } else {
        printf("[NS] ERROR: Secure call failed with status: %d\n", status);
    }
    
    psa_close(handle);
    printf("[NS] Connection closed.\n");

    return 0;
}
