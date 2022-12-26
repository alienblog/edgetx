/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "Arduino.h"
#include "hal/adc_driver.h"

uint8_t adcPins[] = {A0, A1, A2, A3, A4, A4, A6};
static bool arduino_hal_adc_init()
{
  for (int i = 0; i < sizeof(adcPins)/sizeof(adcPins[0]); i++) {
    adcAttachPin(adcPins[i]);
  }
  analogReadResolution(12);
  return true;
}

static bool arduino_hal_adc_start_read()
{
  for (int i = 0; i < sizeof(adcPins)/sizeof(adcPins[0]); i++) {
    //int analogValue = analogRead(adcPins[i]);
    adcValues[i] = analogReadMilliVolts(adcPins[i]);
  }
  return true;
}

static void arduino_hal_adc_wait_completion() {
}

static const etx_hal_adc_driver_t arduino_hal_adc_driver = {
  arduino_hal_adc_init,
  arduino_hal_adc_start_read,
  arduino_hal_adc_wait_completion
};

void adruino_adc_init(void) {
  adcInit(&arduino_hal_adc_driver);
}