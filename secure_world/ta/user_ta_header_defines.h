/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2016-2017, Linaro Limited
 * All rights reserved.
 */

/*
 * The name of this file must not be modified
 */
#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#include <secure_ics_ta.h>

/* This is the unique identifier for your specific ICS vault */
#define TA_UUID \
    { 0x8a1d7f6e, 0x9b2c, 0x4d5e, \
        { 0xaf, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde } }

#define TA_FLAGS                    TA_FLAG_EXEC_DDR
#define TA_STACK_SIZE               (2 * 1024)
#define TA_DATA_SIZE                (32 * 1024)

#define TA_CURRENT_TA_EXT_PROPERTIES \
    { "gp.ta.description", USER_TA_PROP_TYPE_STRING, \
        "Secure ICS Cryptographic Vault" }, \
    { "gp.ta.version", USER_TA_PROP_TYPE_U32, &(const uint32_t){ 0x0010 } }

#endif /* USER_TA_HEADER_DEFINES_H */
