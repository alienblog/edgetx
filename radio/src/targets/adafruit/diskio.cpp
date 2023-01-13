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
#include "FatFs/diskio.h"
#include "FatFs/ff.h"

#include "Arduino.h"
#include "sd_diskio.h"

static bool card_present = true; // default to consider it as present until mount failed

bool SD_CARD_PRESENT(void) {
  return card_present;
}

static uint8_t sddisk = 0xFF;
/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
extern DSTATUS ff_sd_initialize(uint8_t pdrv);
extern DSTATUS ff_sd_status(uint8_t pdrv);
extern DRESULT ff_sd_read(uint8_t pdrv, uint8_t* buffer, DWORD sector, UINT count);
extern DRESULT ff_sd_write(uint8_t pdrv, const uint8_t* buffer, DWORD sector, UINT count);
extern DRESULT ff_sd_ioctl(uint8_t pdrv, uint8_t cmd, void* buff);

DSTATUS disk_initialize (
        BYTE drv                /* Physical drive number (0) */
)
{
  return ff_sd_initialize(drv);
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
        BYTE drv                /* Physical drive number (0) */
)
{
  return ff_sd_status(drv);
}

RTOS_MUTEX_HANDLE spiMutex;
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
        BYTE drv,                       /* Physical drive number (0) */
        BYTE *buff,                     /* Pointer to the data buffer to store read data */
        DWORD sector,           /* Start sector number (LBA) */
        UINT count                      /* Sector count (1..255) */
)
{
  RTOS_LOCK_MUTEX(spiMutex);
  DRESULT res = ff_sd_read(drv, buff, sector, count);
  RTOS_UNLOCK_MUTEX(spiMutex);
  return res;
}

DRESULT disk_write (
        BYTE drv,                       /* Physical drive number (0) */
        const BYTE *buff,       /* Pointer to the data to be written */
        DWORD sector,           /* Start sector number (LBA) */
        UINT count                      /* Sector count (1..255) */
)
{
  RTOS_LOCK_MUTEX(spiMutex);
  DRESULT res = ff_sd_write(drv, buff, sector, count);
  RTOS_UNLOCK_MUTEX(spiMutex);
  return res;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
        BYTE drv,               /* Physical drive number (0) */
        BYTE ctrl,              /* Control code */
        void *buff              /* Buffer to send/receive control data */
)
{
  return ff_sd_ioctl(drv, ctrl, buff);
}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void sdPoll10ms()
{
}

bool _g_FATFS_init = false;
FATFS g_FATFS_Obj __DMA; // this is in uninitialised section !!!

void sdMount()
{
  if (f_mount(&g_FATFS_Obj, "", 1) == FR_OK) {
    // call sdGetFreeSectors() now because f_getfree() takes a long time first time it's called
    _g_FATFS_init = true;
    card_present = true;
  } else {
    card_present = false;
  }
}


// TODO shouldn't be there!
void sdInit(void)
{
  SPI.begin();
  sddisk = sdcard_init(SDCARD_CS_GPIO, &SPI, 4000000);
  if (sddisk != 0xFF) {
    sdMount();
  }
}

void sdDone()
{
  Serial.println("+++++++++c");
}

uint32_t sdMounted()
{
  return _g_FATFS_init;
}

uint32_t sdIsHC()
{
  return (sdcard_type(sddisk) == CARD_SDHC);
}

uint32_t sdGetSpeed()
{
  Serial.println("+++++++++f");
  return 330000;
}
