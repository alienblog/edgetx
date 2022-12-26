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

#ifndef _RTOS_H_
#define _RTOS_H_

#include "definitions.h"

#ifdef __cplusplus
extern "C++" {
#endif

#if defined(SIMU)
  #include <pthread.h>
  #include <semaphore.h>

  #define SIMU_SLEEP_OR_EXIT_MS(x)       simuSleep(x)
  #define RTOS_MS_PER_TICK  1

  typedef pthread_t RTOS_TASK_HANDLE;
  typedef pthread_mutex_t RTOS_MUTEX_HANDLE;

  typedef uint32_t RTOS_FLAG_HANDLE;

  typedef sem_t * RTOS_EVENT_HANDLE;

  extern uint64_t simuTimerMicros(void);
  extern uint8_t simuSleep(uint32_t ms);

  static inline void RTOS_START()
  {
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    simuSleep(x);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    RTOS_WAIT_MS(x * RTOS_MS_PER_TICK);
  }

#ifdef __cplusplus
  static inline void RTOS_CREATE_MUTEX(pthread_mutex_t &mutex)
  {
    mutex = PTHREAD_MUTEX_INITIALIZER;
  }

  static inline void RTOS_LOCK_MUTEX(pthread_mutex_t &mutex)
  {
      pthread_mutex_lock(&mutex);
  }

  static inline void RTOS_UNLOCK_MUTEX(pthread_mutex_t &mutex)
  {
      pthread_mutex_unlock(&mutex);
  }

  template<int SIZE>
  class FakeTaskStack
  {
    public:
      FakeTaskStack()
      {
      }

      void paint()
      {
      }

      uint32_t size()
      {
        return SIZE;
      }

      uint32_t available()
      {
        return SIZE / 2;
      }
  };
  #define RTOS_DEFINE_STACK(name, size) FakeTaskStack<size> name

  #define TASK_FUNCTION(task)           void* task(void *)

  inline void RTOS_CREATE_TASK(pthread_t &taskId, void * (*task)(void *), const char * name)
  {
    pthread_create(&taskId, nullptr, task, nullptr);
#ifdef __linux__
    pthread_setname_np(taskId, name);
#endif
  }

template<int SIZE>
inline void RTOS_CREATE_TASK(pthread_t &taskId, void * (*task)(void *), const char * name, FakeTaskStack<SIZE> &, unsigned size = 0, unsigned priority = 0)
  {
    UNUSED(size);
    UNUSED(priority);
    RTOS_CREATE_TASK(taskId, task, name);
  }

  #define TASK_RETURN()                 return nullptr

  constexpr uint32_t stackAvailable()
  {
    return 500;
  }
#endif  // __cplusplus

  // return 2ms resolution to match CoOS settings
  static inline uint32_t RTOS_GET_TIME(void)
  {
    return (uint32_t)(simuTimerMicros() / 2000);
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (uint32_t)(simuTimerMicros() / 1000);
  }
#elif defined(RTOS_FREERTOS)  // This is the Arduino version of FreeRTOS
  #include "FreeRTOS_entry.h"
  #define RTOS_MS_PER_TICK portTICK_PERIOD_MS

  typedef struct {
    TaskHandle_t rtos_handle;
  } RTOS_TASK_HANDLE;

  typedef SemaphoreHandle_t RTOS_MUTEX_HANDLE;
  typedef EventGroupHandle_t RTOS_FLAG_HANDLE;
  static inline void RTOS_INIT()
  {
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    vTaskDelay((x * configTICK_RATE_HZ) / 1000);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    vTaskDelay(x);
  }
#ifndef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
  static inline void RTOS_START()
  {
    vTaskStartScheduler();
  }

  #define RTOS_CREATE_TASK(taskId, task, name, stackStruct, stackSize, priority)   \
        xTaskCreate(task, name, stackSize, NULL, priority, &taskId.rtos_handle)
  #define RTOS_CREATE_TASK_EX(taskId, task, name, stackStruct, stackSize, priority, core)   \
        xTaskCreate(task, name, stackSize, NULL, priority, &taskId.rtos_handle)
#else
  static inline void RTOS_START() {}

  #define RTOS_CREATE_TASK(taskId, task, name, stackStruct, stackSize, priority)   \
        xTaskCreatePinnedToCore(task, name, stackSize, NULL, priority, &taskId.rtos_handle, 0)
  #define RTOS_CREATE_TASK_EX(taskId, task, name, stackStruct, stackSize, priority, core)   \
        xTaskCreatePinnedToCore(task, name, stackSize, NULL, priority, &taskId.rtos_handle, core)
#endif
  #define RTOS_DEL_TASK(taskId) vTaskDelete(taskId)

#ifdef __cplusplus
  static inline void RTOS_CREATE_MUTEX(RTOS_MUTEX_HANDLE &mutex)
  {
    mutex = xSemaphoreCreateRecursiveMutex();
  }

  static inline void RTOS_LOCK_MUTEX(RTOS_MUTEX_HANDLE &mutex)
  {
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
  }

  static inline void RTOS_UNLOCK_MUTEX(RTOS_MUTEX_HANDLE &mutex)
  {
    xSemaphoreGiveRecursive(mutex);
  }
#endif  // __cplusplus

  #define RTOS_CREATE_FLAG(flag)        flag = xEventGroupCreate()
  #define RTOS_SET_FLAG(flag)           (void)xEventGroupSetBits(flag, 0x01)
  #define RTOS_CLEAR_FLAG(flag)         (void)xEventGroupClearBits(flag, 0x01)
  #define RTOS_WAIT_FLAG(flag,timeout)  ((xEventGroupWaitBits(flag,0x01,pdTRUE,pdFALSE,timeout) & 0x01) == 0)

  static inline void RTOS_ISR_SET_FLAG(RTOS_FLAG_HANDLE flag)
  {
    BaseType_t xResult;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xResult = xEventGroupSetBitsFromISR(flag, 0x01, &xHigherPriorityTaskWoken);
    if( xResult != pdFAIL ) {
      /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
      switch should be requested.  The macro used is port specific and will
      be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
      the documentation page for the port being used. */
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
  }

  static inline uint32_t RTOS_GET_TIME(void)
  {
    return (uint32_t)xTaskGetTickCount();
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (RTOS_GET_TIME() * RTOS_MS_PER_TICK);
  }

#ifdef __cplusplus
  template<int SIZE>
  class TaskStack
  {
    public:
      TaskStack()
      {
      }

      void paint()
      {
      }

      uint32_t size()
      {
        return SIZE * 4;
      }

      uint32_t available()
      {
        return SIZE;
      }

      StackType_t stack[1]; // just to make compiler happy
  };
#endif // __cplusplus
  #define RTOS_DEFINE_STACK(name, size) TaskStack<size> __ALIGNED(8) name

  #define TASK_FUNCTION(task)           void task(void * pdata)
  #define TASK_RETURN()                 return
#elif defined(FREE_RTOS) // This is the edgeTX default FreeRTOS
#ifdef __cplusplus
  extern "C" {
#endif
#if defined(RTOS_FREERTOS)  // This is the Arduino version of FreeRTOS
    #include "FreeRTOS_entry.h"
#else
    #include <FreeRTOS/include/FreeRTOS.h>
    #include <FreeRTOS/include/task.h>
    #include <FreeRTOS/include/semphr.h>
#endif
#ifdef __cplusplus
  }
#endif

  #define RTOS_MS_PER_TICK portTICK_PERIOD_MS

  typedef struct {
    TaskHandle_t rtos_handle;
    StaticTask_t task_struct;
  } RTOS_TASK_HANDLE;

  typedef struct {
    SemaphoreHandle_t rtos_handle;
    StaticSemaphore_t mutex_struct;
  } RTOS_MUTEX_HANDLE;
  
  typedef RTOS_MUTEX_HANDLE RTOS_FLAG_HANDLE;
  
  static inline void RTOS_START()
  {
    vTaskStartScheduler();
  }

  static inline void RTOS_WAIT_MS(uint32_t x)
  {
    if (!x)
      return;
    if ((x = x / RTOS_MS_PER_TICK) < 1)
      x = 1;

    vTaskDelay(x);
  }

  static inline void RTOS_WAIT_TICKS(uint32_t x)
  {
    vTaskDelay(x);
  }

  static inline void _RTOS_CREATE_TASK(RTOS_TASK_HANDLE *h,
                                       TaskFunction_t pxTaskCode,
                                       const char *name,
                                       StackType_t *const puxStackBuffer,
                                       const uint32_t ulStackDepth,
                                       UBaseType_t uxPriority)
  {
#ifndef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
    h->rtos_handle = xTaskCreateStatic(
        pxTaskCode, name, ulStackDepth, 0, uxPriority,
        puxStackBuffer, &h->task_struct);
#else
    h->rtos_handle = xTaskCreateStaticPinnedToCore(
        pxTaskCode, name, ulStackDepth, 0, uxPriority,
        puxStackBuffer, &h->task_struct, 0);
#endif
  }

static inline void _RTOS_CREATE_TASK_EX(RTOS_TASK_HANDLE *h,
                                       TaskFunction_t pxTaskCode,
                                       const char *name,
                                       StackType_t *const puxStackBuffer,
                                       const uint32_t ulStackDepth,
                                       UBaseType_t uxPriority, const BaseType_t xCoreID)
  {
#ifndef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
    h->rtos_handle = xTaskCreateStatic(
        pxTaskCode, name, ulStackDepth, 0, uxPriority,
        puxStackBuffer, &h->task_struct);
#else
    h->rtos_handle = xTaskCreateStaticPinnedToCore(
        pxTaskCode, name, ulStackDepth, 0, uxPriority,
        puxStackBuffer, &h->task_struct, xCoreID);
#endif
  }

  #define RTOS_CREATE_TASK(h,task,name,stackStruct,stackSize,prio) \
    _RTOS_CREATE_TASK(&h,task,name,stackStruct.stack,stackSize,prio)

  #define RTOS_CREATE_TASK_EX(h,task,name,stackStruct,stackSize,prio,core) \
    _RTOS_CREATE_TASK_EX(&h,task,name,stackStruct.stack,stackSize,prio,core)

  static inline void _RTOS_CREATE_MUTEX(RTOS_MUTEX_HANDLE* h)
  {
    h->rtos_handle = xSemaphoreCreateBinaryStatic(&h->mutex_struct);
    xSemaphoreGive(h->rtos_handle);
  }

  #define RTOS_CREATE_MUTEX(handle) _RTOS_CREATE_MUTEX(&handle)

  static inline void _RTOS_LOCK_MUTEX(RTOS_MUTEX_HANDLE* h)
  {
    xSemaphoreTake(h->rtos_handle, portMAX_DELAY);
  }

  #define RTOS_LOCK_MUTEX(handle) _RTOS_LOCK_MUTEX(&handle)

  static inline void _RTOS_UNLOCK_MUTEX(RTOS_MUTEX_HANDLE* h)
  {
    xSemaphoreGive(h->rtos_handle);
  }

  #define RTOS_UNLOCK_MUTEX(handle) _RTOS_UNLOCK_MUTEX(&handle)

  //TODO: replace with FreeRTOS functions
  static inline uint32_t getStackAvailable(void * address, uint32_t size)
  {
    uint32_t * array = (uint32_t *)address;
    uint32_t i = 0;
    while (i < size && array[i] == 0x55555555) {
      i++;
    }
    return i;
  }

  extern int _estack;
  extern int _main_stack_start;
  static inline uint32_t stackSize()
  {
    return ((unsigned char *)&_estack - (unsigned char *)&_main_stack_start) / 4;
  }

  static inline uint32_t stackAvailable()
  {
    return getStackAvailable(&_main_stack_start, stackSize());
  }

  //#define RTOS_CREATE_FLAG(flag)        flag = CoCreateFlag(false, false)
  //#define RTOS_SET_FLAG(flag)           (void)CoSetFlag(flag)
  //#define RTOS_CLEAR_FLAG(flag)         (void)CoClearFlag(flag)

  // returns true if timeout
  static inline bool _RTOS_WAIT_FLAG(RTOS_FLAG_HANDLE* flag, uint32_t timeout)
  {
    return xSemaphoreTake(flag->rtos_handle, timeout * RTOS_MS_PER_TICK)
      == pdFALSE;
  }

  #define RTOS_WAIT_FLAG(flag,timeout) _RTOS_WAIT_FLAG(&flag,timeout)

  static inline void _RTOS_ISR_SET_FLAG(RTOS_FLAG_HANDLE* flag)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Give semaphore back from ISR to trigger a task waiting for it
    xSemaphoreGiveFromISR( flag->rtos_handle, &xHigherPriorityTaskWoken );

    // If xHigherPriorityTaskWoken was set to true we should yield
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }

  #define RTOS_ISR_SET_FLAG(flag) _RTOS_ISR_SET_FLAG(&flag)

#ifdef __cplusplus
  template<int SIZE>
  class TaskStack
  {
    public:
      TaskStack()
      {
      }

      void paint()
      {
        for (uint32_t i=0; i<SIZE; i++) {
          stack[i] = 0x55555555;
        }
      }

      uint32_t size()
      {
        return SIZE * 4;
      }

      uint32_t available()
      {
        return getStackAvailable(stack, SIZE);
      }

      StackType_t stack[SIZE];
  };
#endif // __cplusplus

  static inline TickType_t RTOS_GET_TIME(void)
  {
    return xTaskGetTickCount();
  }

  static inline uint32_t RTOS_GET_MS(void)
  {
    return (RTOS_GET_TIME() * RTOS_MS_PER_TICK);
  }

  // stack must be aligned to 8 bytes otherwise printf for %f does not work!
  #define RTOS_DEFINE_STACK(name, size) TaskStack<size> __ALIGNED(8) name __CCMRAM

  #define TASK_FUNCTION(task)           void task(void *)
  #define TASK_RETURN()                 vTaskDelete(nullptr)

#else // no RTOS

  #error "No RTOS implementation defined"

#endif  // RTOS type

#ifdef __cplusplus
}
#endif

#endif // _RTOS_H_
