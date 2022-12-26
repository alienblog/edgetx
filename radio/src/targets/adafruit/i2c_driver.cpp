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

#include "board.h"

void i2cInit()
{
}

#define I2C_TIMEOUT_MAX 1000
bool I2C_WaitEvent(uint32_t event)
{
  return true;
}

bool I2C_WaitEventCleared(uint32_t event)
{
  return true;
}

#if !defined(BOOT) && !defined(SOFTWARE_VOLUME)
void setScaledVolume(uint8_t volume)
{
}

void setVolume(uint8_t volume)
{
}

int32_t getVolume()
{
  return -1; // TODO
}
#endif
