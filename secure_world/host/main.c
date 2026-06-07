// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 */
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <tee_client_api.h>
#include <secure_ics_ta.h>

#define GATEWAY_IP "10.0.2.2" // qemu default route to the host machine
#define GATEWAY_PORT 5000

int main(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_SECURE_ICS_UUID;
    uint32_t err_origin;

    // init optee context
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) errx(1, "TEEC_InitializeContext failed");

    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) errx(1, "TEEC_Opensession failed");

    // setup TCP socket to host
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        errx(1, "Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(GATEWAY_PORT);

    if (inet_pton(AF_INET, GATEWAY_IP, &serv_addr.sin_addr) <= 0) {
        errx(1, "invalid address or address not supported");
    }

    printf("[host] Attempting to connect to Gateway at %s:%d\n", GATEWAY_IP, GATEWAY_PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        errx(1, "Connection failed");
    }
    printf("[host] Connected to Node.js gateway!\n");

    // continuous verification loop
    char buffer[256] = {0};
    uint8_t hash_output[32] = {0};
    char hex_hash_str[65] = {0};

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, 255);
        
        if (valread <= 0) {
            printf("[host] Gateway disconnected. Exiting.\n");
            break;
        }

        printf("[host <- gateway] Received: %s\n", buffer);

        // prep TEE parameters for hashing
        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);
        op.params[0].tmpref.buffer = buffer;
        op.params[0].tmpref.size = strlen(buffer);
        op.params[1].tmpref.buffer = hash_output;
        op.params[1].tmpref.size = sizeof(hash_output);

        // invoke Secure World 
        res = TEEC_InvokeCommand(&sess, TA_SECURE_ICS_CMD_HASH_TELEMETRY, &op, &err_origin);
        
        if (res == TEEC_SUCCESS) {
            // binary hash to Hex string to send over TCP
            for (int i = 0; i < 32; i++) {
                sprintf(&hex_hash_str[i * 2], "%02x", hash_output[i]);
            }
            
            // send hash back to Node.js gateway
            send(sock, hex_hash_str, strlen(hex_hash_str), 0);
            printf("[host -> gateway] Sent Hash: %s\n", hex_hash_str);
        } else {
            printf("[host] crypto failure in Secure World.\n");
        }
    }

    // cleanup
    close(sock);
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    return 0;
}
