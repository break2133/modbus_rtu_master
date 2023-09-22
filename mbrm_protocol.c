/*
 * mbrm_protocol.c
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

#include <string.h>
#include "mbrm_protocol.h"

static mbrm_protocol_t mbrm_tcb;
static mbrm_protocol_private_t *mbrm_tcb_priv;

#if MBRM_CRC_CODE_MODE == 0
/* CRC High Byte Value Table */
static const uint8_t auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* CRC Low Byte Value Table*/
static const uint8_t auchCRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05,
    0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA,
    0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA,
    0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15,
    0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0,
    0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35,
    0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B,
    0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
    0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27,
    0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64,
    0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE,
    0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7,
    0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
    0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99,
    0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E,
    0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46,
    0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/**
 * @brief Get crc code
 *
 * @param puchMsg
 * @param usDataLen
 * @return uint16_t
 */
static uint16_t _mbrm_get_crc_code(const uint8_t *puchMsg, uint16_t usDataLen)
{
    unsigned uint8_t uchCRCHi = 0xFF;
    unsigned uint8_t uchCRCLo = 0xFF;
    unsigned uIndex;

    while (usDataLen--)
    {
        uIndex = uchCRCHi ^ *puchMsg++;
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
}
#else
/**
 * @brief Get crc code
 *
 * @param puchMsg
 * @param usDataLen
 * @return uint16_t
 */
static uint16_t _mbrm_get_crc_code(const uint8_t *puchMsg, uint16_t usDataLen)
{
    uint8_t i;
    uint16_t crc = 0xffff;

    if (usDataLen == 0)
    {
        usDataLen = 1;
    }
    while (usDataLen--)
    {
        crc ^= *puchMsg;
        for (i = 0; i < 8; i++)
        {
            if (crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
        puchMsg++;
    }
    return (crc);
}
#endif

/**
 * @brief
 * @param status
 */
static void _mbrm_pop_queue(mbrm_queue_status_t status)
{
    uint8_t poped;
    if (mbrm_tcb_priv->queue_tcb.num == 0)
    {
        mbrm_log_e("Queue is empty\r\n");
        return;
    }
    poped = mbrm_tcb_priv->queue_tcb.pop_pos;
    mbrm_log_i("POP queue at %d\r\n", poped);

    mbrm_tcb_priv->queue_tcb.queue[poped].status = status;
    mbrm_tcb_priv->queue_tcb.pop_pos++;
    mbrm_tcb_priv->queue_tcb.pop_pos %= MBRM_COMMUNICATION_QUEUE_MAX_LENTH;
    mbrm_tcb_priv->queue_tcb.num--;
    mbrm_tcb_priv->status = MBRM_PROTOCOL_STATUS_FREE;

    /* Next command sent in the queue */
    if (mbrm_tcb_priv->queue_tcb.num > 0)
    {
        mbrm_tcb_priv->send_data(mbrm_tcb_priv->queue_tcb.pop_pos);
    }

    if (mbrm_tcb_priv->queue_tcb.queue[poped].cfg.pop_sigingal != NULL)
    {
        mbrm_tcb_priv->queue_tcb.queue[poped].cfg.pop_sigingal(poped);
    }
}

/**
 * @brief
 * @param q
 */
static uint8_t _mbrm_push_queue(mbrm_unit_cfg_t *q)
{
    uint8_t pushed;
    uint8_t repeat_max;
    if (mbrm_tcb_priv->queue_tcb.num >= MBRM_COMMUNICATION_QUEUE_MAX_LENTH)
    {
        mbrm_log_e("Queue is full\r\n");
        return 255;
    }
    pushed = mbrm_tcb_priv->queue_tcb.push_pos;
    mbrm_log_i("Push queue at %d\r\n", pushed);
    mbrm_tcb_priv->queue_tcb.queue[pushed].cfg = *q;
    mbrm_tcb_priv->queue_tcb.queue[pushed].status = MBRM_QUEUE_STATUS_WAIT;
    mbrm_tcb_priv->queue_tcb.queue[pushed].repeat = 0;
    repeat_max = mbrm_tcb_priv->queue_tcb.queue[pushed].cfg.repeat_max;
    mbrm_tcb_priv->queue_tcb.queue[pushed].cfg.repeat_max = (repeat_max < 1 || repeat_max > 3) ? 3 : repeat_max;
    mbrm_tcb_priv->queue_tcb.push_pos++;
    mbrm_tcb_priv->queue_tcb.push_pos %= MBRM_COMMUNICATION_QUEUE_MAX_LENTH;

    mbrm_tcb_priv->queue_tcb.num++;
    /* The queue is full, switch to busy. */
    if (mbrm_tcb_priv->queue_tcb.num >= MBRM_COMMUNICATION_QUEUE_MAX_LENTH)
    {
        mbrm_tcb_priv->status = MBRM_PROTOCOL_STATUS_BUSY;
    }
}

/**
 * @brief
 * @param queue_pos
 */
void _mbrm_send_data(uint8_t queue_pos)
{
    mbrm_communication_unit_t *unit = &mbrm_tcb_priv->queue_tcb.queue[queue_pos];
    uint16_t crc_code;
    uint16_t send_data_lenth;

    unit->repeat++;
    if (unit->repeat > unit->cfg.repeat_max)
    {
        mbrm_tcb_priv->pop_queue(MBRM_QUEUE_STATUS_OVER_TIME);
    }

    switch (unit->cfg.cmd)
    {
    case 0x03:
        mbrm_tcb_priv->send_buf[0] = unit->cfg.slave_addr;
        mbrm_tcb_priv->send_buf[1] = unit->cfg.cmd;
        mbrm_tcb_priv->send_buf[2] = unit->cfg.register_addr >> 8;
        mbrm_tcb_priv->send_buf[3] = unit->cfg.register_addr & 0xff;
        mbrm_tcb_priv->send_buf[4] = 0;
        mbrm_tcb_priv->send_buf[5] = unit->cfg.len;
        crc_code = mbrm_tcb_priv->get_crc(mbrm_tcb_priv->send_buf, 6);
        mbrm_tcb_priv->send_buf[6] = crc_code;
        mbrm_tcb_priv->send_buf[7] = crc_code >> 8;

        if (mbrm_tcb_priv->write_cb != NULL)
        {
            mbrm_tcb_priv->write_cb(mbrm_tcb_priv->send_buf, 8);
        }
        break;

    case 0x06:
        mbrm_tcb_priv->send_buf[0] = unit->cfg.slave_addr;
        mbrm_tcb_priv->send_buf[1] = unit->cfg.cmd;
        mbrm_tcb_priv->send_buf[2] = unit->cfg.register_addr >> 8;
        mbrm_tcb_priv->send_buf[3] = unit->cfg.register_addr & 0xff;
        mbrm_tcb_priv->send_buf[4] = unit->cfg.data[0];
        mbrm_tcb_priv->send_buf[5] = unit->cfg.data[1];
        crc_code = mbrm_tcb_priv->get_crc(mbrm_tcb_priv->send_buf, 6);
        mbrm_tcb_priv->send_buf[6] = crc_code;
        mbrm_tcb_priv->send_buf[7] = crc_code >> 8;

        if (mbrm_tcb_priv->write_cb != NULL)
        {
            mbrm_tcb_priv->write_cb(mbrm_tcb_priv->send_buf, 8);
        }
        break;

    case 0x10:
        send_data_lenth = 7 + unit->cfg.len + 2;
        mbrm_tcb_priv->send_buf[0] = unit->cfg.slave_addr;
        mbrm_tcb_priv->send_buf[1] = unit->cfg.cmd;
        mbrm_tcb_priv->send_buf[2] = unit->cfg.register_addr >> 8;
        mbrm_tcb_priv->send_buf[3] = unit->cfg.register_addr & 0xff;
        mbrm_tcb_priv->send_buf[4] = 0;
        mbrm_tcb_priv->send_buf[5] = unit->cfg.len / 2;
        mbrm_tcb_priv->send_buf[6] = unit->cfg.len;
        for (uint8_t i = 0; i < unit->cfg.len; i++)
        {
            mbrm_tcb_priv->send_buf[7 + i] = unit->cfg.data[i];
        }
        crc_code = mbrm_tcb_priv->get_crc(mbrm_tcb_priv->send_buf, send_data_lenth - 2);
        mbrm_tcb_priv->send_buf[send_data_lenth - 2] = crc_code & 0xff;
        mbrm_tcb_priv->send_buf[send_data_lenth - 1] = crc_code >> 8;

        if (mbrm_tcb_priv->write_cb != NULL)
        {
            mbrm_tcb_priv->write_cb(mbrm_tcb_priv->send_buf, send_data_lenth);
        }
        break;

    default:
        mbrm_log_e("Unknown command: 0x%02x\r\n", unit->cfg.cmd);
        break;
    }
}

/**
 * @brief
 * @param data
 * @param len
 */
static void _mbrm_receive(const uint8_t *data, uint16_t len)
{
    RUN_CB(mbrm_tcb_priv->mutex_lock);

    /* 1.Slave addr */
    if (data[0] != mbrm_tcb_priv->queue_tcb.queue[mbrm_tcb_priv->queue_tcb.pop_pos].cfg.slave_addr)
    {
        RUN_CB(mbrm_tcb_priv->mutex_unlock);
        return;
    }

    /* 2.CRC */
    if (mbrm_tcb_priv->get_crc(data, len - 2) != (data[len - 1] << 8 | data[len - 2]))
    {
        RUN_CB(mbrm_tcb_priv->mutex_unlock);
        return;
    }

    /* 3.Cmd */
    if (data[1] != mbrm_tcb_priv->queue_tcb.queue[mbrm_tcb_priv->queue_tcb.pop_pos].cfg.cmd)
    {
        mbrm_tcb_priv->pop_queue(MBRM_QUEUE_STATUS_ERROR);
        RUN_CB(mbrm_tcb_priv->mutex_unlock);
        return;
    }

    switch (data[1])
    {
    case 0x03:
        memcpy(mbrm_tcb_priv->queue_tcb.queue[mbrm_tcb_priv->queue_tcb.pop_pos].cfg.data, data + 3, len - 5);
        break;

    case 0x06:
        /* code */
        break;

    case 0x10:
        /* code */
        break;

    default:
        mbrm_tcb_priv->pop_queue(MBRM_QUEUE_STATUS_ERROR);
        RUN_CB(mbrm_tcb_priv->mutex_unlock);
        return;
    }
    mbrm_tcb_priv->pop_queue(MBRM_QUEUE_STATUS_FINISH);
    RUN_CB(mbrm_tcb_priv->mutex_unlock);

}

/**
 * @brief
 * @param q
 * @return
 */
static uint8_t _mbrm_send_cmd(mbrm_unit_cfg_t *q)
{
    uint8_t ret;
    RUN_CB(mbrm_tcb_priv->mutex_lock);
    ret = mbrm_tcb_priv->push_queue(q);

    /* If the queue is empty before this command, immediately send. */
    if (mbrm_tcb_priv->queue_tcb.num == 1)
    {
        mbrm_tcb_priv->send_data(mbrm_tcb_priv->queue_tcb.pop_pos);
    }
    RUN_CB(mbrm_tcb_priv->mutex_unlock);

    return ret;
}

const mbrm_communication_unit_t *_mbrm_get_unit_in_queue(uint8_t pos)
{
    return &mbrm_tcb_priv->queue_tcb.queue[pos];
}

/**
 * @brief
 * @param
 * @return
 */
static mbrm_protocol_status_t _mbrm_get_status(void)
{
    return mbrm_tcb_priv->status;
}

/**
 * @brief
 * @param cfg
 */
static void _mbrm_init(mbrm_init_cfg *cfg)
{
    mbrm_tcb_priv = (mbrm_protocol_private_t *)mbrm_tcb.priv;

    mbrm_tcb_priv->get_crc = _mbrm_get_crc_code;
    mbrm_tcb_priv->pop_queue = _mbrm_pop_queue;
    mbrm_tcb_priv->push_queue = _mbrm_push_queue;
    mbrm_tcb_priv->send_data = _mbrm_send_data;

    mbrm_tcb_priv->write_cb = cfg->write_cb;
    mbrm_tcb_priv->mutex_lock = cfg->mutex_lock;
    mbrm_tcb_priv->mutex_unlock = cfg->mutex_unlock;
}

static mbrm_protocol_t mbrm_tcb =
{
    .init = _mbrm_init,
    .receive = _mbrm_receive,
    .send_cmd = _mbrm_send_cmd,
    .get_status = _mbrm_get_status,
    .get_unit_in_queue = _mbrm_get_unit_in_queue,
};

/**
 * @brief
 * @param
 * @return
 */
const mbrm_protocol_t *mbrm_get_protocol(void)
{
    return &mbrm_tcb;
}
