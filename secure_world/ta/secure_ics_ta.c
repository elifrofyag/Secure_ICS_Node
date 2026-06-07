#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>
#include <secure_ics_ta.h>

// session management
TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }
void TA_DestroyEntryPoint(void) {}
TEE_Result TA_OpenSessionEntryPoint(uint32_t pt, TEE_Param p[4], void **ctx) {
    (void)pt; (void)p; (void)ctx;
    return TEE_SUCCESS;
}
void TA_CloseSessionEntryPoint(void *ctx) { (void)ctx; }


static TEE_Result hash_telemetry(uint32_t param_types, TEE_Param params[4]) {
    // check whether Normal World sent correct param types
    // expect Param 0 to be input data (memref), Param 1 to be output hash (memref)
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    void *in_buf = params[0].memref.buffer;
    uint32_t in_sz = params[0].memref.size;
    void *out_buf = params[1].memref.buffer;
    uint32_t *out_sz = &params[1].memref.size;

    // ensure output buffer is large enough for a sha256 hash 32 bytes
    if (*out_sz < 32) {
        *out_sz = 32;
        return TEE_ERROR_SHORT_BUFFER;
    }

    //init crypto operation
    TEE_OperationHandle op;
    TEE_Result res;

    res = TEE_AllocateOperation(&op, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0);
    if (res != TEE_SUCCESS)
        return res;

    // hash
    res = TEE_DigestDoFinal(op, in_buf, in_sz, out_buf, out_sz);
    
    // clean
    TEE_FreeOperation(op);

    return res;
}

// command router
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
                                      uint32_t param_types, TEE_Param params[4]) {
    (void)sess_ctx;

    switch (cmd_id) {
    case TA_SECURE_ICS_CMD_HASH_TELEMETRY:
        return hash_telemetry(param_types, params);
    default:
        return TEE_ERROR_BAD_PARAMETERS;
    }
}
