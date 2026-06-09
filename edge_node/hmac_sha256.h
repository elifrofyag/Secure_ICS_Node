#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <stddef.h>
#include <stdint.h>


#define SHA256_BLOCK_SIZE 64
#define SHA256_HASH_SIZE 32

// Internal SHA-256 Context
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} SHA256_CTX;

// Public HMAC-SHA256 API
void hmac_sha256(
    const uint8_t *key, size_t key_len,
    const uint8_t *data, size_t data_len,
    uint8_t *mac_out
);

#endif 