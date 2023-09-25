/*
 * mbrm_protocol.h
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

#ifndef _MODBUS_RTU_MASTER_MBRM_PROTOCOL_H_
#define _MODBUS_RTU_MASTER_MBRM_PROTOCOL_H_

#include <stdint.h>
#include <stddef.h>
#include "mbrm_cfg.h"

#define RUN_CB(_cb_)  do{if(_cb_ != NULL) {_cb_();}}while (0)

typedef enum
{
    MBRM_PROTOCOL_STATUS_FREE = 0,
    MBRM_PROTOCOL_STATUS_BUSY,
} mbrm_protocol_status_t;

typedef enum
{
    MBRM_QUEUE_STATUS_FINISH = 0,
    MBRM_QUEUE_STATUS_WAIT,
    MBRM_QUEUE_STATUS_OVER_TIME,
    MBRM_QUEUE_STATUS_ERROR,
} mbrm_queue_status_t;

typedef struct
{
    uint8_t slave_addr;
    uint8_t cmd;
    uint16_t register_addr;
    uint8_t len;
    uint8_t repeat_max;
    uint16_t over_time;
    uint8_t *data;
    void (*pop_sigingal)(uint8_t poped);
    void *user_param;
} mbrm_unit_cfg_t;

typedef struct
{
    uint8_t repeat;
    mbrm_queue_status_t status;
    mbrm_unit_cfg_t cfg;
} mbrm_communication_unit_t;

typedef struct
{
    uint8_t pop_pos;
    uint8_t push_pos;
    uint16_t num;
    mbrm_communication_unit_t queue[MBRM_COMMUNICATION_QUEUE_MAX_LENTH];
} mbrm_queue_t;

typedef struct
{
    void (*write_cb)(const uint8_t *, uint16_t);
    void (*mutex_lock)(void);
    void (*mutex_unlock)(void);
    void (*timer_start_cb)(uint16_t over_time);
    void (*timer_stop_cb)(void);
    void *(*malloc_hock)(size_t size);
    void (*free_hock)(void *ptr);
} mbrm_init_cfg;

typedef struct
{
    uint8_t send_buf[256];
    mbrm_protocol_status_t status;
    mbrm_queue_t queue_tcb;
    uint16_t (*get_crc)(const uint8_t *, uint16_t);
    void (*pop_queue)(mbrm_queue_status_t);
    uint8_t (*push_queue)(mbrm_unit_cfg_t *q);
    void (*write_cb)(const uint8_t *, uint16_t);
    void (*mutex_lock)(void);
    void (*mutex_unlock)(void);
    void (*timer_start_cb)(uint16_t over_time);
    void (*timer_stop_cb)(void);
    void (*send_data)(uint8_t);
} mbrm_protocol_private_t;

typedef struct
{
    /* PRIVATE */
    char priv[sizeof(mbrm_protocol_private_t)];

    /* PUBLIC */
    void (*init)(const mbrm_init_cfg *);
    uint8_t (*send_cmd)(mbrm_unit_cfg_t *q);
    void (*receive)(const uint8_t *, uint16_t);
    void (*timer_over)(void);
    mbrm_protocol_status_t (*get_status)(void);
    const mbrm_communication_unit_t *(*get_unit_in_queue)(uint8_t);
} mbrm_protocol_t;

const mbrm_protocol_t *mbrm_get_protocol(void);

#endif /* _MODBUS_RTU_MASTER_MBRM_PROTOCOL_H_ */
