/*
 * mbrm_cfg.h
 * Copyright (C) 2023 fan.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _MODBUS_RTU_MASTER_MBRM_CFG_H_
#define _MODBUS_RTU_MASTER_MBRM_CFG_H_

#include <stdio.h>

/**
 * Switch of log(def = CLOSE).
 */
#define MBRM_LOG_SWITCH 0
#if MBRM_LOG_SWITCH
    #define mbrm_log_i(...) printf(__VA_ARGS__)
    #define mbrm_log_w(...) printf(__VA_ARGS__)
    #define mbrm_log_e(...) printf(__VA_ARGS__)
#else
    #define mbrm_log_i(...)
    #define mbrm_log_w(...)
    #define mbrm_log_e(...)
#endif

/**
 * Selete CRC code mode.
 * 0(def): Look-Up Table
 * other: Calculation
 */
#define MBRM_CRC_CODE_MODE 0

/**
 * Maximum length of communication queue(def: 5; max: 254).
 */
#define MBRM_COMMUNICATION_QUEUE_MAX_LENTH 5

/**
 * Maximum of slave device(def: 5).
 */
#define MBRM_DEVICE_MAX_NUM 5

/**
 * Length of slave device's name(def: 5).
 */
#define MBRM_DEVICE_NAME_LENTH 5

#endif /* _MODBUS_RTU_MASTER_MBRM_CFG_H_ */
