/*
 * Copyright (C) OpenTX
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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_task.h"
#include "rom/ets_sys.h"
#include "rom/crc.h"
#include "esprc.h"
#include "esprc_packet.h"

#include "pulses_esp32.h"

static void* BtPowerUPInit(uint8_t module)
{
  (void)module;
  return 0;
}

static void BtPowerUPDeInit(void* context)
{
}

static void BtPowerUPSetupPulses(void* context, int16_t* channels, uint8_t nChannels)
{
  // nothing to do
}

static void BtPowerUPSendPulses(void* context)
{
}

static int BtPowerUPGetByte(void* context, uint8_t* data)
{
return 0;
}

static void BtPowerUPProcessData(void* context, uint8_t data, uint8_t* buffer, uint8_t* len)
{
}

#include "hal/module_driver.h"

const etx_module_driver_t BtPowerUPDriver = {
  .protocol = PROTOCOL_CHANNELS_ESPNOW,
  .init = BtPowerUPInit,
  .deinit = BtPowerUPDeInit,
  .setupPulses = BtPowerUPSetupPulses,
  .sendPulses = BtPowerUPSendPulses,
  .getByte = BtPowerUPGetByte,
  .processData = BtPowerUPProcessData,
};