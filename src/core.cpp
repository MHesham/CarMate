#include "common.h"
#include <Arduino.h>
#include <timers.h>
#include "carmate.h"
#include "board.h"
#include "tasks_config.h"

TimerHandle_t xStatusLedTimer = NULL;
SemaphoreHandle_t xSpiLock = NULL;
SemaphoreHandle_t xI2cLock = NULL;
SemaphoreHandle_t xCarReadingLock = NULL;
SemaphoreHandle_t xWeatherReadingLock = NULL;
EventGroupHandle_t xSystemEvents = NULL;

/* Defined in main.c. */
void vConfigureTimerForRunTimeStats( void )
{
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  // Set CS31 bit so timer runs at clock speed / 8 = 2KHz
  TCCR3B |= (1 << CS31);
}

unsigned long vGetTimerForRunTimeStats( void )
{
  return TCNT3;
}

void prvStatusLedTimerCallback( TimerHandle_t xTimer )
{
  digitalWrite(boardSTATUS_LED_PIN, !digitalRead(boardSTATUS_LED_PIN));
}

void vCoreInit(void)
{
  digitalWrite(boardSTATUS_LED_PIN, LOW);
  pinMode(boardSTATUS_LED_PIN, OUTPUT);

  digitalWrite(boardERROR_HOOK_LED_PIN, LOW);
  pinMode(boardERROR_HOOK_LED_PIN, OUTPUT);

  Serial.begin(115200);
	/* Initialise the UART. */
	while (!Serial)
  {

  }

  xStatusLedTimer = xTimerCreate("StatusLedTimer", STATUS_LED_TIMER_PERIOD_TICKS,
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

  xSystemEvents = xEventGroupCreate();
  configASSERT(xSystemEvents);
}
