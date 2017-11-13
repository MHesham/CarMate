#include "common.h"

TimerHandle_t xStatusLedTimer = NULL;
SemaphoreHandle_t xSerialLock = NULL;
SemaphoreHandle_t xSpiLock = NULL;
SemaphoreHandle_t xI2cLock = NULL;

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
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO,
      // MICRO, YUN, and other 32u4 based boards.
  }

  digitalWrite(boardSTATUS_LED_PIN, LOW);
  pinMode(boardSTATUS_LED_PIN, OUTPUT);

  digitalWrite(boardERROR_HOOK_LED_PIN, LOW);
  pinMode(boardERROR_HOOK_LED_PIN, OUTPUT);

  xStatusLedTimer = xTimerCreate("StatusLedTimer", STATUS_LED_TIMER_PERIOD_TICKS,
                                 pdTRUE, 0, prvStatusLedTimerCallback);
  configASSERT(xStatusLedTimer);
  BaseType_t xStatus = xTimerStart(xStatusLedTimer, 0);
  configASSERT(xStatus != pdFAIL);

  xSerialLock = xSemaphoreCreateMutex();
  configASSERT(xSerialLock);
  xSemaphoreGive(xSerialLock);

  xSpiLock = xSemaphoreCreateMutex();
  configASSERT(xSpiLock);
  xSemaphoreGive(xSpiLock);

  xI2cLock = xSemaphoreCreateMutex();
  configASSERT(xI2cLock);
  xSemaphoreGive(xI2cLock);

  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << "Core initialized\n";
  xSemaphoreGive(xSerialLock);
}
