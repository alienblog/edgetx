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

// This file uses a file on SPIFFS to simulate EEPROM

#include "board.h"
#include "Arduino.h"
#include "FS.h"
#include "SPIFFS.h"

#define EEPROM_FILE "/eeprom.dat"

static FS &flashfs = SPIFFS;

int eepromInit(void) {
  if(!SPIFFS.begin(true)){
        Serial.println("files sytem Mount Failed");
        return -1;
  }
  if (!flashfs.exists(EEPROM_FILE)) {
    File file = flashfs.open(EEPROM_FILE, FILE_WRITE, true);
    if (file) {
      file.close();
    } else {
      return -1;
    }
  }
  return 0;
}

void eepromReadBlock(uint8_t * buffer, size_t address, size_t size)
{
  File file = flashfs.open(EEPROM_FILE);
  if (file) {
    file.seek(address, SeekSet);
    file.read(buffer, size);
  }
}

void eepromStartRead(uint8_t * buffer, size_t address, size_t size) {
  eepromReadBlock(buffer, address, size);
}

void eepromBlockErase(uint32_t address) {
  File file = flashfs.open(EEPROM_FILE, FILE_WRITE);
  if (file) {
    file.seek(address, SeekSet);
    uint8_t byte = 0xFF;
    for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
      file.write(&byte, 1);
    }
  }
}
/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  buffer : pointer to the buffer containing the data to be
  *   written to the EEPROM.
  * @param  address : EEPROM's internal address to write to.
  * @param  size : number of bytes to write to the EEPROM.
  * @retval None
  */
void eepromWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  File file = flashfs.open(EEPROM_FILE, FILE_WRITE);
  if (file) {
    file.seek(address, SeekSet);
    file.write(buffer, size);
  }
}

void eepromStartWrite(uint8_t * buffer, size_t address, size_t size) {
  eepromWriteBlock(buffer, address, size);
}

uint8_t eepromIsTransferComplete()
{
  return 1;
}

uint8_t eepromReadStatus() {
  return 1;
}