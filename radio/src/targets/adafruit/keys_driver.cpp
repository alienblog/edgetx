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
#include <Adafruit_MCP23X17.h>

static Adafruit_MCP23X17 mcp;
static Adafruit_MCP23X17 mcp1;
static RTOS_MUTEX_HANDLE keyMutex;
uint32_t readKeys()
{
  uint32_t result = 0;
  RTOS_LOCK_MUTEX(keyMutex);
  uint8_t mask = (1 << BUTTONS_ON_GPIOA) - 1;
  uint8_t gpioA = mcp.readGPIOA();
  result |= (gpioA ^ mask) & mask;

  gpioA = mcp1.readGPIOA();
  RTOS_UNLOCK_MUTEX(keyMutex);
  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  return result;
}

bool trimDown(uint8_t idx)
{
  return readTrims() & (1 << idx);
}

bool keyDown()
{
  return readKeys() || readTrims();
}

/* TODO common to ARM */
void readKeysAndTrims()
{
  uint8_t index = 0;
  uint32_t keys_input = readKeys();
  for (unsigned i = 1; i != unsigned(1 << TRM_BASE); i <<= 1) {
    keys[index++].input(keys_input & i);
  }

  uint32_t trims_input = readTrims();
  for (uint8_t i = 1; i != uint8_t(1 << 8); i <<= 1) {
    keys[index++].input(trims_input & i);
  }

#if defined(PWR_BUTTON_PRESS)
  if ((keys_input || trims_input || pwrPressed()) && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
#else
  if ((keys_input || trims_input) && (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
#endif
    // on keypress turn the light on
    resetBacklightTimeout();
  }
}

#if !defined(BOOT)
uint32_t switchState(uint8_t index)
{
  uint32_t xxx = 0;

  // TRACE("switch %d => %d", index, xxx);
  return xxx;
}
#endif

void keysInit()
{
  RTOS_CREATE_MUTEX(keyMutex);
  mcp.begin_I2C(MCP23XXX_ADDR, &Wire);
  mcp1.begin_I2C(MCP23XXX_ADDR + 1, &Wire);

  for (int i = 0; i < BUTTONS_ON_GPIOA; i++) {
    mcp.pinMode(i, INPUT_PULLUP);
  }
}

void INTERNAL_MODULE_ON(void) {

}
void INTERNAL_MODULE_OFF(void) {
}

bool IS_INTERNAL_MODULE_ON(void) {
  return false;
}
