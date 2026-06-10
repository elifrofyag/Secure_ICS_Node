#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>
#include <stdio.h>
#include <secure_ics_ta.h>

static const char *DEVICE_PSK = "HMAC_PresharedSecretKeyPicoOptee";
static const char *ENCLAVE_PRIVATE_KEY = "opteeIsolatedSignaturePrivateKey";

TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }
void TA_DestroyEntryPoint(void) {}
TEE_Result TA_OpenSessionEntryPoint(uint32_t pt, TEE_Param p[4], void **ctx) {
    (void)pt; (void)p; (void)ctx; return TEE_SUCCESS;
}
void TA_CloseSessionEntryPoint(void *ctx) { (void)ctx; }

static TEE_Result do_hmac(const char *key, uint32_t key_len, void *in, uint32_t in_len, uint8_t *out, uint32_t *out_len) {
    TEE_OperationHandle op;
    TEE_ObjectHandle key_obj;
    TEE_Attribute attr;
    TEE_Result res;


    // Use 512 bits (SHA-256 block size) for strict compliance
    res = TEE_AllocateOperation(&op, TEE_ALG_HMAC_SHA256, TEE_MODE_MAC, 512);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_AllocateOperation failed: 0x%x", res);
        return res;
    }

    res = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, 512, &key_obj);
    if (res != TEE_SUCCESS) { 
        EMSG("TEE_AllocateTransientObject failed: 0x%x", res);
        TEE_FreeOperation(op); 
        return res; 
    }

    TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, key_len);
    res = TEE_PopulateTransientObject(key_obj, &attr, 1);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_PopulateTransientObject failed: 0x%x", res);
        TEE_FreeTransientObject(key_obj);
        TEE_FreeOperation(op);
        return res;
    }

    res = TEE_SetOperationKey(op, key_obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SetOperationKey failed: 0x%x", res);
        TEE_FreeTransientObject(key_obj);
        TEE_FreeOperation(op);
        return res;
    }

    TEE_MACInit(op, NULL, 0);
    res = TEE_MACComputeFinal(op, in, in_len, out, out_len);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_MACComputeFinal failed: 0x%x", res);
    }

    TEE_FreeTransientObject(key_obj);
    TEE_FreeOperation(op);
    return res;
}

static TEE_Result verify_and_sign(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INOUT,
                                               TEE_PARAM_TYPE_NONE);
    // Explicitly log parameter mismatches
    if (param_types != exp_param_types) {
        EMSG("Param mismatch! Expected: 0x%x, Got: 0x%x", exp_param_types, param_types);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    void *payload = params[0].memref.buffer;
    uint32_t payload_sz = params[0].memref.size;
    char *client_hmac_hex = params[1].memref.buffer;
    void *out_signature = params[2].memref.buffer;
    uint32_t *out_sz = &params[2].memref.size;

    if (*out_sz < 32) {
        EMSG("Output buffer too small: %u", *out_sz);
        return TEE_ERROR_SHORT_BUFFER;
    }

    uint8_t computed_mac[32];
    uint32_t mac_sz = sizeof(computed_mac);
    TEE_Result res = do_hmac(DEVICE_PSK, strlen(DEVICE_PSK), payload, payload_sz, computed_mac, &mac_sz);
    if (res != TEE_SUCCESS) return res;

    char computed_hex[65] = {0};
    for(int i = 0; i < 32; i++) {
        snprintf(&computed_hex[i * 2], 3, "%02x", computed_mac[i]);
    }

    if (memcmp(computed_hex, client_hmac_hex, 64) != 0) {
        EMSG("SECURITY ALERT: HMAC mismatch!");
        EMSG("Pico sent: %s", client_hmac_hex);
        EMSG("Enclave computed: %s", computed_hex);
        return TEE_ERROR_SECURITY;
    }

    res = do_hmac(ENCLAVE_PRIVATE_KEY, strlen(ENCLAVE_PRIVATE_KEY), payload, payload_sz, out_signature, out_sz);
    return res;
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
                                      uint32_t param_types, TEE_Param params[4]) {
    (void)sess_ctx;
    switch (cmd_id) {
    case TA_SECURE_ICS_CMD_HASH_TELEMETRY:
        return verify_and_sign(param_types, params);
    default:
        EMSG("Unknown Command ID: %u", cmd_id);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}