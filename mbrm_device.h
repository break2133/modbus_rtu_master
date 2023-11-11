/*
 * mbrm_device.h
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

#ifndef _MODBUS_RTU_MASTER_MBRM_DEVICE_H_
#define _MODBUS_RTU_MASTER_MBRM_DEVICE_H_

#include "mbrm_cfg.h"
#include "mbrm_protocol.h"

typedef enum
{
    MBRM_DEV_16_12 = 0,
    MBRM_DEV_16_21,
} mbrm_device_16_mode_t;

typedef enum
{
    MBRM_DEV_32_1234 = 0,
    MBRM_DEV_32_2143,
    MBRM_DEV_32_3412,
    MBRM_DEV_32_4321,
} mbrm_device_32_mode_t;

typedef enum
{
    MBRM_TYPE_16 = 0,
    MBRM_TYPE_32,
} mbrm_device_type_t;

typedef union
{
    uint8_t data8[2];
    uint16_t data16;
} mbrm_device_u16_t;

typedef union
{
    uint8_t data8[4];
    uint32_t data32;
} mbrm_device_u32_t;

typedef struct
{
    uint8_t cmd;
    uint16_t register_addr;
    uint16_t num;
    mbrm_device_type_t type;
    void *data;
} mbrm_device_cmd_t;

typedef struct
{
    char name[MBRM_DEVICE_NAME_LENTH];
    uint8_t slave_addr;
    uint8_t repeat_max;
    uint16_t over_time;

    mbrm_device_16_mode_t mode_16;
    mbrm_device_32_mode_t mode_32;
    mbrm_device_cmd_t *cmd_list;
} mbrm_device_info_t;

typedef struct mbrm_device
{
    mbrm_device_info_t info;
    struct mbrm_device *prev;
    struct mbrm_device *next;
} mbrm_device_t;

typedef struct
{
    mbrm_device_t *pdev;
    mbrm_device_cmd_t *pcmd;
    void(*complete_cb)(mbrm_queue_status_t status, void *data);
} mbrm_device_cmd_info_t;

typedef struct
{
    uint8_t send_len;
    mbrm_device_t *devs;
    int (*insert)(mbrm_device_info_t *info);
    void (*remove)(mbrm_device_t *p);
    void (*pop_sigingal)(uint8_t poped);
    void (*send_protocol)(mbrm_device_cmd_info_t *cmd_info);
    void *(*malloc_hock)(size_t size);
    void (*free_hock)(void *ptr);
} mbrm_device_class_private_t;

typedef struct
{
    /* PRIVATE */
    char priv[sizeof(mbrm_device_class_private_t)];

    /* PUBLIC */
    const mbrm_protocol_t *protocol;
    void (*init)(const mbrm_init_cfg *cfg);
    int (*dev_register)(mbrm_device_info_t *info);
    int (*dev_detach)(char *name);
    int (*dev_send_cmd)(char *name, int cmd, void(*complete_cb)(mbrm_queue_status_t status, void *data));
    int (*dev_set_data)(char *name, int cmd, void *data);
} mbrm_device_class_t;

const mbrm_device_class_t *get_mbrm_devive_obj(void);

#endif /* _MODBUS_RTU_MASTER_MBRM_DEVICE_H_ */
