/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"
#include "flyskyHallStick_driver.h"
#include "esp32_rmt_pulse.h"

unsigned char HallCmd[264];

static void flysky_hall_stick_decode_cb(rmt_ctx_t *ctx,
                                        rmt_rx_done_event_data_t *rxdata);

STRUCT_HALL HallProtocol = {0};
STRUCT_HALL HallProtocolTx = {0};
signed short hall_raw_values[FLYSKY_HALL_CHANNEL_COUNT];
unsigned short hall_adc_values[FLYSKY_HALL_CHANNEL_COUNT];

/* crc16 implementation according to CCITT standards */
const unsigned short CRC16Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

// const uint8_t sticks_mapping[4] = { 0 /*STICK1*/,  1/*STICK2*/, 2/*STICK3*/,
// 3 /*STICK4*/};

unsigned short calc_crc16(void *pBuffer, unsigned char BufferSize)
{
  unsigned short crc16;
  crc16 = 0xffff;
  while (BufferSize) {
    crc16 = (crc16 << 8) ^
            CRC16Table[((crc16 >> 8) ^ (*(unsigned char *)pBuffer)) & 0x00ff];
    pBuffer = (void *)((unsigned char *)pBuffer + 1);
    BufferSize--;
  }
  return crc16;
}

uint16_t get_flysky_hall_adc_value(uint8_t ch)
{
  if (ch >= FLYSKY_HALL_CHANNEL_COUNT) {
    return 0;
  }
  uint16_t res = (19000 - hall_adc_values[ch]);
  return res;  // TODO-feather
  // return 2*FLYSKY_OFFSET_VALUE - hall_adc_values[ch];
}

void HallSendBuffer(uint8_t *buffer, uint32_t count) {}

uint8_t HallGetByte(uint8_t *byte) { return 0; }

void reset_hall_stick(void)
{
  unsigned short crc16 = 0xffff;

  HallCmd[0] = FLYSKY_HALL_PROTOLO_HEAD;
  HallCmd[1] = 0xD1;
  HallCmd[2] = 0x01;
  HallCmd[3] = 0x01;

  crc16 = calc_crc16(HallCmd, 4);

  HallCmd[4] = crc16 & 0xff;
  HallCmd[5] = crc16 >> 8 & 0xff;

  HallSendBuffer(HallCmd, 6);
}

static StaticTask_t rx_task_buf;
EXT_RAM_BSS_ATTR static rmt_ctx_t ctxbuf;
void flysky_hall_stick_init()
{
  /* Why use RMT instead of UART?
   * 1. for fun
   * 2. don't need to worry about finding a pin for TX which is not used.
   */
  ctxbuf.task_struct = &rx_task_buf;
  esp32_rmt_rx_init(&ctxbuf, FLYSKY_UART_RX_PIN, 128, 25,
                    flysky_hall_stick_decode_cb,
                    70,  // 14bytes of 8n1, total maximum of 70 pulses (each
                         // byte max 5 low/high pair)
                    815000,
                    900  // FlySky hall stick communicates at a little bit less
                         // that 1Mbps. So minimum pulse is around 1000ns
  );
  reset_hall_stick();
}

static int parse_ps_state = 0;
void Parse_Character(STRUCT_HALL *hallBuffer, unsigned char ch)
{
  if (parse_ps_state != 0) return;
  parse_ps_state = 1;
  switch (hallBuffer->status) {
    case GET_START: {
      if (FLYSKY_HALL_PROTOLO_HEAD == ch) {
        hallBuffer->head = FLYSKY_HALL_PROTOLO_HEAD;
        hallBuffer->status = GET_ID;
        hallBuffer->msg_OK = 0;
      }
      break;
    }
    case GET_ID: {
      hallBuffer->hallID.ID = ch;
      hallBuffer->status = GET_LENGTH;
      break;
    }
    case GET_LENGTH: {
      hallBuffer->length = ch;
      hallBuffer->dataIndex = 0;
      hallBuffer->status = GET_DATA;
      if (0 == hallBuffer->length) {
        hallBuffer->status = GET_CHECKSUM;
        hallBuffer->checkSum = 0;
      }
      break;
    }
    case GET_DATA: {
      hallBuffer->data[hallBuffer->dataIndex++] = ch;
      if (hallBuffer->dataIndex >= hallBuffer->length) {
        hallBuffer->checkSum = 0;
        hallBuffer->dataIndex = 0;
        hallBuffer->status = GET_STATE;
      }
      break;
    }
    case GET_STATE: {
      hallBuffer->checkSum = 0;
      hallBuffer->dataIndex = 0;
      hallBuffer->status = GET_CHECKSUM;
    }
    case GET_CHECKSUM: {
      hallBuffer->checkSum |= ch << ((hallBuffer->dataIndex++) * 8);
      if (hallBuffer->dataIndex >= 2) {
        hallBuffer->dataIndex = 0;
        hallBuffer->status = CHECKSUM;
      } else {
        break;
      }
    }
    case CHECKSUM: {
      if (hallBuffer->checkSum ==
          calc_crc16((void *)&hallBuffer->head, hallBuffer->length + 3)) {
        hallBuffer->msg_OK = 1;
        goto Label_restart;
      } else {
        goto Label_error;
      }
    }
  }

  goto exit;

Label_error:
Label_restart:
  hallBuffer->status = GET_START;
exit:
  parse_ps_state = 0;
  return;
}

void flysky_hall_process_byte(uint8_t byte)
{
  HallProtocol.index++;

  Parse_Character(&HallProtocol, byte);
  if (HallProtocol.msg_OK) {
    HallProtocol.msg_OK = 0;
    HallProtocol.stickState = HallProtocol.data[HallProtocol.length - 1];

    switch (HallProtocol.hallID.hall_Id.receiverID) {
      case TRANSFER_DIR_TXMCU:
        if (HallProtocol.hallID.hall_Id.packetID ==
            FLYSKY_HALL_RESP_TYPE_VALUES) {
          memcpy(hall_raw_values, HallProtocol.data, sizeof(hall_raw_values));
          for (uint8_t channel = 0; channel < 4; channel++) {
            hall_adc_values[channel] =
                FLYSKY_OFFSET_VALUE + hall_raw_values[channel];
          }
        }
        break;
    }
    globalData.flyskygimbals = true;
  }
}

static void flysky_hall_stick_decode_cb(rmt_ctx_t *ctx,
                                        rmt_rx_done_event_data_t *rxdata)
{
  int b = -1;

  uint16_t data = 0;
  for (size_t i = 0; i < rxdata->num_symbols; i++) {
    uint32_t low = rxdata->received_symbols[i].duration0;
    uint32_t high = rxdata->received_symbols[i].duration1;

    uint32_t low_bits = low / 43;
    uint32_t high_bits = high / 43;

    b += low_bits;
    data |= ((1 << high_bits) - 1) << b;
    b += high_bits;

    if (b >= 8) {
      flysky_hall_process_byte((uint8_t)(data & 0xFF));
      data = 0;
      b = -1;
    }
  }
}
