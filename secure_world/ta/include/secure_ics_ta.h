#ifndef TA_SECURE_ICS_H
#define TA_SECURE_ICS_H

/* * UUID must match exactly what is in user_ta_header_defines.h 
 */
#define TA_SECURE_ICS_UUID \
    { 0x8a1d7f6e, 0x9b2c, 0x4d5e, \
        { 0xaf, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde } }

/*  command ID to trigger the SHA-256 hash on incoming telemetry */
#define TA_SECURE_ICS_CMD_HASH_TELEMETRY    0

#endif /* TA_SECURE_ICS_H */
