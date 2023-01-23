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
/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

extern void adruino_adc_init(void);
extern void flysky_hall_stick_init();

HardwareOptions hardwareOptions;

#if defined(__cplusplus)
extern "C" {
#endif

//#include "usb_dcd_int.h"
//#include "usb_bsp.h"
#if defined(__cplusplus)
}
#endif

//HardwareOptions hardwareOptions;

void watchdogInit(unsigned int duration)
{
}

#if defined(SPORT_UPDATE_PWR_GPIO)
void sportUpdateInit()
{
}

void sportUpdatePowerOn()
{
}

void sportUpdatePowerOff()
{
}

void sportUpdatePowerInit()
{
  if (g_eeGeneral.sportUpdatePower == 1)
    sportUpdatePowerOn();
  else
    sportUpdatePowerOff();
}
#endif

void loop() {
  //display.display();
}

extern RTOS_MUTEX_HANDLE spiMutex;
void boardInit()
{
  disableCore0WDT();
  disableCore1WDT();

RTOS_CREATE_MUTEX(spiMutex);
#ifdef ARDUINO_FEATHER_F405
#else // default ESP32V2
  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER, HIGH);
#endif

#if defined(DEBUG)
  serialSetMode(SP_AUX1, UART_MODE_DEBUG);
  serialInit(SP_AUX1, UART_MODE_DEBUG);
#endif
#if !defined(COLORLCD)
  lcdInit();
#endif

  keysInit();

  initWiFi();

  lv_init();
  /* Initialize SPI or I2C bus used by the drivers */
  lvgl_driver_init();

  flysky_hall_stick_init();
  init5msTimer();
  init2MhzTimer();
  adruino_adc_init();
}

void boardOff()
{
  // this function must not return!
}

#if defined(AUDIO_SPEAKER_ENABLE_GPIO)
void initSpeakerEnable()
{
}

void enableSpeaker()
{
}

void disableSpeaker()
{
}
#endif

#if defined(HEADPHONE_TRAINER_SWITCH_GPIO)
void initHeadphoneTrainerSwitch()
{
}

void enableHeadphone()
{
}

void enableTrainer()
{
}
#endif

#if defined(JACK_DETECT_GPIO)
void initJackDetect(void)
{
}
#endif

int usbPlugged() {
  return 0;// TODO-feather
}

void enableVBatBridge() {

}
void disableVBatBridge() {

}
bool isVBatBridgeEnabled() {
  return false;
}
#ifndef ARDUINO_FEATHER_F405
void NVIC_SystemReset(void) {}
#endif