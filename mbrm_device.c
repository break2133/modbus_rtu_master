/*
 * mbrm_device.c
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

#include "mbrm_device.h"
#include "stdlib.h"
#include "string.h"

static mbrm_device_class_t mbrm_dev;
static mbrm_device_class_private_t *mbrm_dev_priv;

static int _mbrm_dev_insert(mbrm_device_info_t *info)
{
    mbrm_device_t *p = (mbrm_device_t *)mbrm_dev_priv->malloc_hock(sizeof(mbrm_device_t));
    if (p == NULL)
    {
        mbrm_log_e("Memory alloc fail.\r\n");
        return 2;
    }

    p->info = *info;

    if (mbrm_dev_priv->devs == NULL)
    {
        p->next = p;
        p->prev = p;
        mbrm_dev_priv->devs = p;
    }
    else
    {
        p->next = mbrm_dev_priv->devs;
        p->prev = mbrm_dev_priv->devs->prev;
        p->next->prev = p;
        p->prev->next = p;
    }
    mbrm_log_i("Device \"%s\" insert succeed.\r\n", p->info.name);
    return 0;
}

static void _mbrm_dev_remove(mbrm_device_t *p)
{

    if (p->next == p)
    {
        /* Only one. */
        mbrm_dev_priv->free_hock(p);
        mbrm_dev_priv->devs = NULL;
    }
    else
    {
        /* Tow or more. */
        if (mbrm_dev_priv->devs == p)
        {
            mbrm_dev_priv->devs = p->next;
        }
        p->next->prev = p->prev;
        p->prev->next = p->next;

        mbrm_dev_priv->free_hock(p);
    }
}

/**
 * @brief
 * @param
 * @return 0 Succeed; -1: Parameter err; 1: Not found target; 2: Device list is null.
 */
static int _mbrm_dev_detach(char *name)
{
    if ((name == NULL))
    {
        mbrm_log_e("device_detach: Parameter err.\r\n");
        return -1;
    }

    if (mbrm_dev_priv->devs != NULL)
    {
        mbrm_device_t *pdev = mbrm_dev_priv->devs;
        do
        {
            if (strncmp(pdev->info.name, name, MBRM_DEVICE_NAME_LENTH) == 0)
            {
                mbrm_dev_priv->remove(pdev);
                return 0;
            }
            pdev = pdev->next;
        }
        while (pdev != mbrm_dev_priv->devs);

        mbrm_log_w("device_detach: Not found target.\r\n");
        return 1;
    }
    else
    {
        mbrm_log_w("device_detach: Device list is null.\r\n");
        return 2;
    }
}

/**
 * @brief
 * @param
 * @return 0 Succeed; -1: parameter err; 1: Target already exists; 2: Memory alloc fail.
 */
static int _mbrm_dev_register(mbrm_device_info_t *info)
{
    if (info == NULL)
    {
        mbrm_log_e("device_register: parameter err.\r\n");
        return -1;
    }
    if ((info->name == NULL) || strlen(info->name) >= MBRM_DEVICE_NAME_LENTH)
    {
        mbrm_log_e("device_register: parameter err.\r\n");
        return -1;
    }

    if (mbrm_dev_priv->devs != NULL)
    {
        mbrm_device_t *pdev = mbrm_dev_priv->devs;
        do
        {
            if (strncmp(pdev->info.name, info->name, MBRM_DEVICE_NAME_LENTH) == 0)
            {
                mbrm_log_w("device_detach: Target already exists.\r\n");
                return 1;
            }
            pdev = pdev->next;
        }
        while (pdev != mbrm_dev_priv->devs);
    }

    return mbrm_dev_priv->insert(info);
}

static void _mbrm_dev_pop_sigingal(uint8_t poped)
{
    const mbrm_communication_unit_t *unit = mbrm_dev.protocol->get_unit_in_queue(poped);
    mbrm_device_cmd_info_t *cmd_info = (mbrm_device_cmd_info_t *)unit->cfg.user_param;
    int i = 0;
    uint8_t *read_data = (uint8_t *)cmd_info->pcmd->data;

    if (cmd_info->pcmd->cmd != 0x03 || unit->status != MBRM_QUEUE_STATUS_FINISH)
    {
        goto complete;
    }

    /* cmd_info->pcmd->cmd == 0x03 && unit->status == MBRM_QUEUE_STATUS_FINISH */
    if (cmd_info->pcmd->type == MBRM_TYPE_16)
    {
        mbrm_device_u16_t *data_16 = (mbrm_device_u16_t *)unit->cfg.data;
        switch (cmd_info->pdev->info.mode_16)
        {
        case MBRM_DEV_16_12:
            for (i = 0; i < cmd_info->pcmd->num; i++)
            {
                read_data[i * 2] = data_16->data8[1];
                read_data[i * 2 + 1] = data_16->data8[0];
                data_16++;
            }
            break;
        case MBRM_DEV_16_21:
            memcpy(cmd_info->pcmd->data, unit->cfg.data, 2 * cmd_info->pcmd->num);
            break;
        default:
            break;
        }
    }
    else if (cmd_info->pcmd->type == MBRM_TYPE_32)
    {
        mbrm_device_u32_t *data_32 = (mbrm_device_u32_t *)unit->cfg.data;
        switch (cmd_info->pdev->info.mode_32)
        {
        case MBRM_DEV_32_1234:
            for (i = 0; i < cmd_info->pcmd->num; i++)
            {
                read_data[i * 4] = data_32->data8[3];
                read_data[i * 4 + 1] = data_32->data8[2];
                read_data[i * 4 + 2] = data_32->data8[1];
                read_data[i * 4 + 3] = data_32->data8[0];
                data_32++;
            }
            break;
        case MBRM_DEV_32_2143:
            for (i = 0; i < cmd_info->pcmd->num; i++)
            {
                read_data[i * 4] = data_32->data8[2];
                read_data[i * 4 + 1] = data_32->data8[3];
                read_data[i * 4 + 2] = data_32->data8[0];
                read_data[i * 4 + 3] = data_32->data8[1];
                data_32++;
            }
            break;
        case MBRM_DEV_32_3412:
            for (i = 0; i < cmd_info->pcmd->num; i++)
            {
                read_data[i * 4] = data_32->data8[1];
                read_data[i * 4 + 1] = data_32->data8[0];
                read_data[i * 4 + 2] = data_32->data8[3];
                read_data[i * 4 + 3] = data_32->data8[2];
                data_32++;
            }
            break;
        case MBRM_DEV_32_4321:
            memcpy(cmd_info->pcmd->data, unit->cfg.data, 4 * cmd_info->pcmd->num);
            break;
        default:
            break;
        }
    }

complete:
    switch (unit->status)
    {
    case MBRM_QUEUE_STATUS_FINISH:
        mbrm_log_i("MBRM_QUEUE_STATUS_FINISH\r\n");
        break;
    case MBRM_QUEUE_STATUS_OVER_TIME:
        mbrm_log_w("MBRM_QUEUE_STATUS_OVER_TIME\r\n");
        break;
    case MBRM_QUEUE_STATUS_ERROR:
        mbrm_log_w("MBRM_QUEUE_STATUS_ERROR\r\n");
        break;

    default:
        break;
    }

    if (cmd_info->complete_cb != NULL)
    {
        cmd_info->complete_cb(unit->status, cmd_info->pcmd->data);
        cmd_info->complete_cb = NULL;
    }
    mbrm_dev_priv->free_hock(unit->cfg.data);
    mbrm_dev_priv->free_hock(cmd_info);
}

static void _mbrm_dev_send_protocol(mbrm_device_cmd_info_t *cmd_info)
{
    uint8_t *buf = NULL;
    mbrm_device_t *pdev = cmd_info->pdev;
    mbrm_device_cmd_t *pcmd = cmd_info->pcmd;

    switch (pcmd->type)
    {
    case MBRM_TYPE_16:
        mbrm_dev_priv->send_len = pcmd->num;
        mbrm_device_u16_t *send_data16 = (mbrm_device_u16_t *)pcmd->data;
        buf = (uint8_t *)mbrm_dev_priv->malloc_hock(2 * mbrm_dev_priv->send_len);

        for (size_t i = 0; i < pcmd->num; i++)
        {
            switch (pdev->info.mode_16)
            {
            case MBRM_DEV_16_12:
                buf[i * 2] = send_data16->data8[1];
                buf[i * 2 + 1] = send_data16->data8[0];
                break;
            case MBRM_DEV_16_21:
                buf[i * 2] = send_data16->data8[0];
                buf[i * 2 + 1] = send_data16->data8[1];
                break;
            default:
                break;
            }
            send_data16++;
        }
        break;

    case MBRM_TYPE_32:
        mbrm_dev_priv->send_len = 2 * pcmd->num;
        mbrm_device_u32_t *send_data32 = (mbrm_device_u32_t *)pcmd->data;
        buf = (uint8_t *)mbrm_dev_priv->malloc_hock(2 * mbrm_dev_priv->send_len);

        for (size_t i = 0; i < pcmd->num; i++)
        {
            switch (pdev->info.mode_32)
            {
            case MBRM_DEV_32_1234:
                buf[i * 4] = send_data32->data8[3];
                buf[i * 4 + 1] = send_data32->data8[2];
                buf[i * 4 + 2] = send_data32->data8[1];
                buf[i * 4 + 3] = send_data32->data8[0];
                break;
            case MBRM_DEV_32_2143:
                buf[i * 4] = send_data32->data8[2];
                buf[i * 4 + 1] = send_data32->data8[3];
                buf[i * 4 + 2] = send_data32->data8[0];
                buf[i * 4 + 3] = send_data32->data8[1];
                break;
            case MBRM_DEV_32_3412:
                buf[i * 4] = send_data32->data8[1];
                buf[i * 4 + 1] = send_data32->data8[0];
                buf[i * 4 + 2] = send_data32->data8[3];
                buf[i * 4 + 3] = send_data32->data8[2];
                break;

            case MBRM_DEV_32_4321:
                buf[i * 4] = send_data32->data8[0];
                buf[i * 4 + 1] = send_data32->data8[1];
                buf[i * 4 + 2] = send_data32->data8[2];
                buf[i * 4 + 3] = send_data32->data8[3];
                break;
            default:
                break;
            }
            send_data32++;
        }
        break;

    default:
        break;
    }

    mbrm_unit_cfg_t cfg =
    {
        .cmd = pcmd->cmd,
        .slave_addr = pdev->info.slave_addr,
        .register_addr = pcmd->register_addr,
        .data = buf,
        .len = mbrm_dev_priv->send_len,
        .repeat_max = pdev->info.repeat_max,
        .over_time = pdev->info.over_time,
        .pop_sigingal = mbrm_dev_priv->pop_sigingal,
        .user_param = cmd_info,
    };
    mbrm_dev.protocol->send_cmd(&cfg);
}

/**
 * @brief
 * @param
 * @return 0 Succeed; -1: parameter err; 1: Target not found; 2: Memory alloc fail.
 */
static int _mbrm_dev_send_cmd(char *name, int cmd, void(*complete_cb)(mbrm_queue_status_t status, void *data))
{
    if (name == NULL)
    {
        mbrm_log_e("device_send_cmd: parameter err.\r\n");
        return -1;
    }
    if (mbrm_dev_priv->devs == NULL)
    {
        mbrm_log_w("device_send_cmd: Target not found.\r\n");
        return 1;
    }

    mbrm_device_t *pdev = mbrm_dev_priv->devs;
    do
    {
        if (strncmp(pdev->info.name, name, MBRM_DEVICE_NAME_LENTH) == 0)
        {
            mbrm_device_cmd_t *pcmd = &pdev->info.cmd_list[cmd];
            mbrm_device_cmd_info_t *cmd_info = (mbrm_device_cmd_info_t *)mbrm_dev_priv->malloc_hock(sizeof(mbrm_device_cmd_info_t));
            cmd_info->pdev = pdev;
            cmd_info->pcmd = pcmd;
            cmd_info->complete_cb = complete_cb;
            mbrm_dev_priv->send_protocol(cmd_info);

            return 0;
        }
        pdev = pdev->next;
    }
    while (pdev != mbrm_dev_priv->devs);

    mbrm_log_w("device_send_cmd: Target not found.\r\n");
    return 1;
}

static void _mbrm_dev_init(const mbrm_init_cfg *cfg)
{
    mbrm_dev_priv = (mbrm_device_class_private_t *)mbrm_dev.priv;

    mbrm_dev.protocol = mbrm_get_protocol();
    mbrm_dev.protocol->init(cfg);

    mbrm_dev_priv->insert = _mbrm_dev_insert;
    mbrm_dev_priv->remove = _mbrm_dev_remove;
    mbrm_dev_priv->send_protocol = _mbrm_dev_send_protocol;
    mbrm_dev_priv->pop_sigingal = _mbrm_dev_pop_sigingal;
    if (cfg->malloc_hock == NULL || cfg->free_hock == NULL)
    {
        mbrm_dev_priv->malloc_hock = malloc;
        mbrm_dev_priv->free_hock = free;
    }
    else
    {
        mbrm_dev_priv->malloc_hock = cfg->malloc_hock;
        mbrm_dev_priv->free_hock = cfg->free_hock;
    }
}

static int _mbrm_dev_set_data(char *name, int cmd, void *data)
{
    if (name == NULL)
    {
        mbrm_log_e("dev_set_data: parameter err.\r\n");
        return -1;
    }
    if (mbrm_dev_priv->devs == NULL)
    {
        mbrm_log_w("dev_set_data: Target not found.\r\n");
        return 1;
    }

    mbrm_device_t *pdev = mbrm_dev_priv->devs;
    do
    {
        if (strncmp(pdev->info.name, name, MBRM_DEVICE_NAME_LENTH) == 0)
        {
            mbrm_device_cmd_t *pcmd = &pdev->info.cmd_list[cmd];

            if (pcmd->type == MBRM_TYPE_16)
            {
                memcpy(pcmd->data, data, pcmd->num * 2);
            }
            else
            {
                memcpy(pcmd->data, data, pcmd->num * 4);
            }

            return 0;
        }
        pdev = pdev->next;
    }
    while (pdev != mbrm_dev_priv->devs);

    mbrm_log_w("dev_set_data: Target not found.\r\n");
    return 1;
}

static mbrm_device_class_t mbrm_dev =
{
    .init = _mbrm_dev_init,
    .dev_detach = _mbrm_dev_detach,
    .dev_register = _mbrm_dev_register,
    .dev_send_cmd = _mbrm_dev_send_cmd,
    .dev_set_data = _mbrm_dev_set_data,
};

const mbrm_device_class_t *get_mbrm_devive_obj(void)
{
    return &mbrm_dev;
}
