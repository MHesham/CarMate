
#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include "carmate.h"
#include "board.h"
#include "tasks_config.h"
#include "utils.h"

TimerHandle_t xStatusLedTimer = NULL;
SemaphoreHandle_t xSpiLock = NULL;
SemaphoreHandle_t xI2cLock = NULL;
SemaphoreHandle_t xCarReadingLock = NULL;
SemaphoreHandle_t xWeatherReadingLock = NULL;
EventGroupHandle_t xSystemEvents = NULL;

void prvStatusLedTimerCallback(TimerHandle_t xTimer) {
  digitalWrite(boardSTATUS_LED_PIN, !digitalRead(boardSTATUS_LED_PIN));
}

void vCoreInit(void) {
  digitalWrite(boardSTATUS_LED_PIN, LOW);
  pinMode(boardSTATUS_LED_PIN, OUTPUT);

  digitalWrite(boardERROR_HOOK_LED_PIN, LOW);
  pinMode(boardERROR_HOOK_LED_PIN, OUTPUT);

  xSystemEvents = xEventGroupCreate();
  configASSERT(xSystemEvents);

  vSerialConsoleInit();
  vRegisterCLICommands();

  xStatusLedTimer =
      xTimerCreate("StatusLedTimer", tskcfgSTATUS_LED_TIMER_PERIOD_TICKS,
                   pdTRUE, 0, prvStatusLedTimerCallback);
  configASSERT(xStatusLedTimer);
  BaseType_t xStatus = xTimerStart(xStatusLedTimer, 0);
  configASSERT(xStatus != pdFAIL);

  xSpiLock = xSemaphoreCreateMutex();
  configASSERT(xSpiLock);

  xI2cLock = xSemaphoreCreateMutex();
  configASSERT(xI2cLock);

  xCarReadingLock = xSemaphoreCreateMutex();
  configASSERT(xCarReadingLock);

  xWeatherReadingLock = xSemaphoreCreateMutex();
  configASSERT(xWeatherReadingLock);
}
