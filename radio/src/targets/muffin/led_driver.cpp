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
#include "i2c_driver.h"
#include "mcp23017.h"

#define LED_R_BIT 0x08
#define LED_G_BIT 0x10
#define LED_B_BIT 0x04

void ledInit()
{
#if 0  // TODO-feather
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

#if defined(LED_GREEN_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_GREEN_GPIO_PIN;
  GPIO_Init(LED_GREEN_GPIO, &GPIO_InitStructure);
#endif

#if defined(LED_RED_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_RED_GPIO_PIN;
  GPIO_Init(LED_RED_GPIO, &GPIO_InitStructure);
#endif

#if defined(LED_BLUE_GPIO)
  GPIO_InitStructure.GPIO_Pin = LED_BLUE_GPIO_PIN;
  GPIO_Init(LED_BLUE_GPIO, &GPIO_InitStructure);
#endif

#if defined(FUNCTION_SWITCHES)
  RCC_AHB1PeriphClockCmd(FS_RCC_AHB1Periph, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = FSLED_GPIO_PIN_1 | FSLED_GPIO_PIN_2 | FSLED_GPIO_PIN_3 | FSLED_GPIO_PIN_4 | FSLED_GPIO_PIN_5 | FSLED_GPIO_PIN_6;
  GPIO_Init(FSLED_GPIO, &GPIO_InitStructure);
#endif
#endif
}

#if defined(FUNCTION_SWITCHES)
constexpr uint32_t fsLeds[] = {FSLED_GPIO_PIN_1, FSLED_GPIO_PIN_2,
                               FSLED_GPIO_PIN_3, FSLED_GPIO_PIN_4,
                               FSLED_GPIO_PIN_5, FSLED_GPIO_PIN_6};

void fsLedOff(uint8_t index)
{
  // #if 0 // TODO-feather GPIO_FSLED_GPIO_OFF(FSLED_GPIO, fsLeds[index]);
}

void fsLedOn(uint8_t index)
{
  // #if 0 // TODO-feather GPIO_FSLED_GPIO_ON(FSLED_GPIO, fsLeds[index]);
}
#endif

void ledOff()
{
  uint8_t gpioB[1] = {0};
  i2c_register_read(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1), gpioB,
                    sizeof(gpioB));
  i2c_register_write_byte(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1),
                          gpioB[0] | LED_R_BIT | LED_G_BIT | LED_B_BIT);
}

void ledRed()
{
  ledOff();
#if defined(LED_RED_GPIO)
  uint8_t gpioB[1] = {0};
  i2c_register_read(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1), gpioB,
                    sizeof(gpioB));
  i2c_register_write_byte(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1),
                          gpioB[0] & ~LED_R_BIT);
#endif
}

void ledGreen()
{
  ledOff();
#if defined(LED_GREEN_GPIO)
  uint8_t gpioB[1] = {0};
  i2c_register_read(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1), gpioB,
                    sizeof(gpioB));
  i2c_register_write_byte(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1),
                          gpioB[0] & ~LED_G_BIT);
#endif
}

void ledBlue()
{
  ledOff();
#if defined(LED_BLUE_GPIO)
  uint8_t gpioB[1] = {0};
  i2c_register_read(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1), gpioB,
                    sizeof(gpioB));
  i2c_register_write_byte(MCP0_ADDR, MCP_REG_ADDR(MCP23XXX_GPIO, 1),
                          gpioB[0] & ~LED_B_BIT);
#endif
}
