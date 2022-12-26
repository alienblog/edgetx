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
#include "mixer_scheduler.h"
#include "Arduino.h"
#include "FreeRTOS_entry.h"

#if defined(ARDUINO_FEATHER_F405)
#include "HardwareTimer.h"
static HardwareTimer *mixer_timer = NULL;
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
static hw_timer_t *mixer_timer = NULL;
#endif

#if defined(ARDUINO_FEATHER_F405)
static void interrupt_mixer(void) {
  mixer_timer->pause();
  size_t usec = 2 * getMixerSchedulerPeriod() - 1;
  mixer_timer->setOverflow(usec, MICROSEC_FORMAT);
  mixer_timer->resume();
}
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
static void IRAM_ATTR interrupt_mixer() {
  timerAlarmDisable(mixer_timer);
  size_t usec = 2 * getMixerSchedulerPeriod() - 1;
  timerAlarmWrite(mixer_timer, usec, true);
  timerAlarmEnable(mixer_timer);
}
#endif

// Start scheduler with default period
void mixerSchedulerStart()
{
  size_t usec = 2 * getMixerSchedulerPeriod() - 1;
#if defined(ARDUINO_FEATHER_F405)
  mixer_timer = new HardwareTimer(TIM4);
  mixer_timer->setOverflow(usec, MICROSEC_FORMAT);
  mixer_timer->attachInterrupt(interrupt_mixer);
  mixer_timer->resume();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  mixer_timer = timerBegin(2, 80, true);
  timerAttachInterrupt(mixer_timer, &interrupt_mixer, true);
  timerAlarmWrite(mixer_timer, usec, true);
  timerAlarmEnable(mixer_timer);
#endif
}

void mixerSchedulerStop()
{
#if defined(ARDUINO_FEATHER_F405)
  mixer_timer->pause();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  timerAlarmDisable(mixer_timer);
#endif
}

void mixerSchedulerResetTimer()
{
#if defined(ARDUINO_FEATHER_F405)
  mixer_timer->pause();
  mixer_timer->setCount(0, MICROSEC_FORMAT);
  mixer_timer->resume();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  timerAlarmDisable(mixer_timer);
  timerWrite(mixer_timer, 0);
  timerAlarmEnable(mixer_timer);
#endif
}

void mixerSchedulerEnableTrigger()
{
#if defined(ARDUINO_FEATHER_F405)
  mixer_timer->resume();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  timerAlarmEnable(mixer_timer);
#endif
}

void mixerSchedulerDisableTrigger()
{
#if defined(ARDUINO_FEATHER_F405)
  mixer_timer->pause();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  timerAlarmDisable(mixer_timer);
#endif
}
