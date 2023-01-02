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
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_RA8875.h"

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

bool lcdInitFinished = false;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer, const rect_t &copy_area)
{
  for (int y = 0; y < copy_area.h; y++) {
    uint16_t * p = &buffer[copy_area.w * y];
    tft.drawPixels(p, copy_area.w, copy_area.x, y);
  }
}

/*
  Proper method for turning of LCD module. It must be used,
  otherwise we might damage LCD crystals in the long run!
*/
void lcdOff()
{
}

/*
  Starts LCD initialization routine. It should be called as
  soon as possible after the reset because LCD takes a lot of
  time to properly power-on.

  Make sure that delay_ms() is functional before calling this function!
*/
void lcdInit()
{
  if (!lcdInitFinished) {
    if (!tft.begin(RA8875_480x272)) {
      Serial.println("RA8875 Not Found!");
      while (1);
    }
    lcdInitFinished = true;
  }
  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  Serial.print("(");
  Serial.print(tft.width());
  Serial.print(", ");
  Serial.print(tft.height());
  Serial.println(")");
  tft.graphicsMode();                 // go back to graphics mode
  tft.fillScreen(RA8875_BLACK);
  tft.graphicsMode();

  lcdSetFlushCb(startLcdRefresh);
}

void lcdSetRefVolt(uint8_t val)
{
}

void DMAWait()
{
}

void DMACopyBitmap(uint16_t *dest, uint16_t destw, uint16_t desth, uint16_t x,
                   uint16_t y, const uint16_t *src, uint16_t srcw,
                   uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                   uint16_t h)
{
  for (int i = 0; i < h; i++) {
    memcpy(dest + (y + i) * destw + x, src + (srcy + i) * srcw + srcx, 2 * w);
  }
}

// 'src' has ARGB4444
// 'dest' has RGB565
void DMACopyAlphaBitmap(uint16_t *dest, uint16_t destw, uint16_t desth,
                        uint16_t x, uint16_t y, const uint16_t *src,
                        uint16_t srcw, uint16_t srch, uint16_t srcx,
                        uint16_t srcy, uint16_t w, uint16_t h)
{
  for (coord_t line = 0; line < h; line++) {
    uint16_t *p = dest + (y + line) * destw + x;
    const uint16_t *q = src + (srcy + line) * srcw + srcx;
    for (coord_t col = 0; col < w; col++) {
      uint8_t alpha = *q >> 12;
      uint8_t red =
          ((((*q >> 8) & 0x0f) << 1) * alpha + (*p >> 11) * (0x0f - alpha)) /
          0x0f;
      uint8_t green = ((((*q >> 4) & 0x0f) << 2) * alpha +
                       ((*p >> 5) & 0x3f) * (0x0f - alpha)) /
                      0x0f;
      uint8_t blue = ((((*q >> 0) & 0x0f) << 1) * alpha +
                      ((*p >> 0) & 0x1f) * (0x0f - alpha)) /
                     0x0f;
      *p = (red << 11) + (green << 5) + (blue << 0);
      p++;
      q++;
    }
  }
}

// 'src' has A8/L8?
// 'dest' has RGB565
void DMACopyAlphaMask(uint16_t *dest, uint16_t destw, uint16_t desth,
                      uint16_t x, uint16_t y, const uint8_t *src, uint16_t srcw,
                      uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w,
                      uint16_t h, uint16_t fg_color)
{
  RGB_SPLIT(fg_color, red, green, blue);

  for (coord_t line = 0; line < h; line++) {
    uint16_t *p = dest + (y + line) * destw + x;
    const uint8_t *q = src + (srcy + line) * srcw + srcx;
    for (coord_t col = 0; col < w; col++) {
      uint16_t opacity = *q >> 4;  // convert to 4 bits (stored in 8bit for DMA)
      uint8_t bgWeight = OPACITY_MAX - opacity;
      RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
      uint16_t r = (bgRed * bgWeight + red * opacity) / OPACITY_MAX;
      uint16_t g = (bgGreen * bgWeight + green * opacity) / OPACITY_MAX;
      uint16_t b = (bgBlue * bgWeight + blue * opacity) / OPACITY_MAX;
      *p = RGB_JOIN(r, g, b);
      p++;
      q++;
    }
  }
}

static TouchState internalTouchState = {0};
struct TouchState getInternalTouchState() {
  return internalTouchState;
}

struct TouchState touchPanelRead() {
  if (tft.touched())
  {
    uint16_t tx, ty;
    float xScale = 1024.0F/tft.width();
    float yScale = 1024.0F/tft.height();
    tft.touchRead(&tx, &ty);

    internalTouchState.x = (uint16_t)(tx/xScale);
    internalTouchState.y = (uint16_t)(ty/yScale);
    internalTouchState.event = TE_DOWN;
  } else {
    internalTouchState.event = TE_UP;
  }
  return internalTouchState;
}

bool touchPanelEventOccured() {
  return tft.touched();
}

bool touchPanelInit(void) {
  if (!lcdInitFinished) {
    if (!tft.begin(RA8875_480x272)) {
      Serial.println("RA8875 Not Found!");
      while (1);
    }
    lcdInitFinished = true;
  }
  tft.touchEnable(true);

  return true;
}