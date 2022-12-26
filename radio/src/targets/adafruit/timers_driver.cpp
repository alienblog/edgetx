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
#include "FreeRTOS_entry.h"

#if defined(ARDUINO_FEATHER_F405)
#include "HardwareTimer.h"
static HardwareTimer *MyTim5ms = NULL;
static HardwareTimer *MyTim2Mhz = NULL;
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
static hw_timer_t *MyTim5ms = NULL;
static hw_timer_t *MyTim2Mhz = NULL;
#endif
static SemaphoreHandle_t sem5ms;

#if defined(ARDUINO_FEATHER_F405)
static void interrupt5ms(void)
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
static void IRAM_ATTR interrupt5ms()
#endif
{
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(sem5ms, &pxHigherPriorityTaskWoken);
}

static void task5ms(void * pdata) {
  static uint8_t pre_scale;       // Used to get 10 Hz counter

  while (true) {
    if (xSemaphoreTake(sem5ms, portMAX_DELAY)) {
      ++pre_scale;

      if (pre_scale == 2) {
        pre_scale = 0;
        per10ms();
      }
    }
  }
}

// Start TIMER at 2000000Hz
void init2MhzTimer()
{
#if defined(ARDUINO_FEATHER_F405)
  MyTim2Mhz = new HardwareTimer(TIM3);
  uint32_t clock_freq_hz = MyTim2Mhz->getTimerClkFreq();
  MyTim2Mhz->setPrescaleFactor((2000000 / clock_freq_hz) - 1);
  MyTim2Mhz->resume();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  MyTim2Mhz = timerBegin(1, 40, true); // 2MHz
  timerStart(MyTim2Mhz);
#endif
}

uint16_t getTmr2MHz() {
#if defined(ARDUINO_FEATHER_F405)
  return MyTim2Mhz->getCount(TICK_FORMAT);
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  return timerRead(MyTim2Mhz);
#endif
}

tmr10ms_t get_tmr10ms() {
#if defined(ARDUINO_FEATHER_F405)
  return (tmr10ms_t)MyTim5ms->getCount(TICK_FORMAT) / 2;
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  return (tmr10ms_t)(timerRead(MyTim2Mhz) / 20000); // 2MHz => 100Hz
#endif
}

#define TIM5MS_STACK_SIZE (1024 * 2)
RTOS_DEFINE_STACK(task5ms_stack, TIM5MS_STACK_SIZE);
RTOS_TASK_HANDLE taskId5ms;
// Start TIMER at 200Hz
void init5msTimer()
{
  sem5ms = xSemaphoreCreateBinary();

  RTOS_CREATE_TASK_EX(taskId5ms,task5ms,"5ms timer",task5ms_stack,TIM5MS_STACK_SIZE,5,TMR_5MS_CORE);  // TODO-feather priority
#if defined(ARDUINO_FEATHER_F405)
  MyTim5ms = new HardwareTimer(TIM2);
  MyTim5ms->setOverflow(200, HERTZ_FORMAT);
  MyTim5ms->attachInterrupt(interrupt5ms);
  MyTim5ms->resume();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  MyTim5ms = timerBegin(0, 80, true);
  timerAttachInterrupt(MyTim5ms, &interrupt5ms, true);
  timerAlarmWrite(MyTim5ms, 5000, true); // 200Hz
  timerAlarmEnable(MyTim5ms);
#endif
}

void stop5msTimer()
{
#if defined(ARDUINO_FEATHER_F405)
  MyTim5ms->pause();
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  timerAlarmDisable(MyTim5ms);
#endif
}
