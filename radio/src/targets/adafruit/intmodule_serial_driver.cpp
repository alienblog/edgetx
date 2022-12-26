/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
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
#include "intmodule_serial_driver.h"

typedef Fifo<uint8_t, INTMODULE_FIFO_SIZE> RxFifo;
static RxFifo intmoduleFifo;

struct IntmoduleCtx
{
  RxFifo* rxFifo;
#if 0
  const stm32_usart_t* usart;
#endif
};

static uint8_t * intmoduleTxBufferData;
static volatile uint8_t intmoduleTxBufferRemaining;

uint8_t intmoduleTxBufferSend(uint8_t* data)
{
  if (intmoduleTxBufferRemaining) {
    *data = *(intmoduleTxBufferData++);
    intmoduleTxBufferRemaining--;
    return true;
  }

  // buffer is empty
  return false;
}

static etx_serial_callbacks_t intmodule_driver = {
  intmoduleTxBufferSend,
  nullptr, nullptr
};

// TODO: move this somewhere else
static void intmoduleFifoReceive(uint8_t data)
{
  intmoduleFifo.push(data);
}

void intmoduleStop()
{
  INTERNAL_MODULE_OFF();
#if 0
  stm32_usart_deinit(&intmoduleUSART);

  // reset callbacks
  intmodule_driver.on_receive = nullptr;
  intmodule_driver.on_error = nullptr;
#endif
}

static const IntmoduleCtx intmoduleCtx = {
  .rxFifo = &intmoduleFifo,
#if 0
  .usart = &intmoduleUSART,
#endif
};

static void intmoduleStop(void* ctx)
{
  (void)ctx;
  intmoduleStop();
}

void* intmoduleSerialStart(const etx_serial_init* params)
{
  if (!params) return nullptr;
#if 0
  // TODO: sanity check parameters
  //  - the UART seems to block when initialised with baudrate = 0

  // init callbacks
#if !defined(INTMODULE_RX_DMA)
  intmodule_driver.on_receive = intmoduleFifoReceive;
#else
  intmodule_driver.on_receive = nullptr;
#endif
  intmodule_driver.on_error = nullptr;

  stm32_usart_init(&intmoduleUSART, params);

  if (params->rx_enable && intmoduleUSART.rxDMA) {
    stm32_usart_init_rx_dma(&intmoduleUSART, intmoduleFifo.buffer(), intmoduleFifo.size());
  }  
  intmoduleCtx.rxFifo->clear();
#endif
  return (void*)&intmoduleCtx;
}
#if 0
#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
extern "C" void INTMODULE_USART_IRQHandler(void)
{
  stm32_usart_isr(&intmoduleUSART, &intmodule_driver);
}
#endif
void intmoduleSendByte(void* ctx, uint8_t byte)
{
#if 0
  auto modCtx = (IntmoduleCtx*)ctx;
  stm32_usart_send_byte(modCtx->usart, byte);
#endif
}

void intmoduleSendBuffer(void* ctx, const uint8_t * data, uint8_t size)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (size == 0)
    return;

#if !defined(INTMODULE_DMA_STREAM)
  intmoduleTxBufferData = (uint8_t *)data;
  intmoduleTxBufferRemaining = size;
#endif
#if 0
  stm32_usart_send_buffer(modCtx->usart, data, size);
#endif
}

void intmoduleWaitForTxCompleted(void* ctx)
{
  while (intmoduleTxBufferRemaining > 0);
}

static int intmoduleGetByte(void* ctx, uint8_t* data)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return -1;
  return modCtx->rxFifo->pop(*data);
}

static void intmoduleClearRxBuffer(void* ctx)
{
  auto modCtx = (IntmoduleCtx*)ctx;
  if (!modCtx->rxFifo) return;
  modCtx->rxFifo->clear();
}

const etx_serial_driver_t IntmoduleSerialDriver = {
  .init = intmoduleSerialStart,
  .deinit = intmoduleStop,
  .sendByte = intmoduleSendByte,
  .sendBuffer = intmoduleSendBuffer,
  .waitForTxCompleted = intmoduleWaitForTxCompleted,
  .getByte = intmoduleGetByte,
  .clearRxBuffer = intmoduleClearRxBuffer,
  .getBaudrate = nullptr,
  .setReceiveCb = nullptr,
  .setBaudrateCb = nullptr,
};
