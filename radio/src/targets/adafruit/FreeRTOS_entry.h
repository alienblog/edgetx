//#include <Arduino.h>
#ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#elif ARDUINO_FEATHER_F405
#include "STM32FreeRTOS.h"
#endif