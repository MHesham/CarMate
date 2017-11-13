#ifndef __COMMON_H__
#define __COMMON_H__

extern void vAssertCalled( const char * const pcFileName,
                           const char * const pcFunctionName,
                           unsigned long ulLine );

/* Define configASSERT() to call vAssertCalled() if the assertion fails.  The assertion
has failed if the value of the parameter passed into configASSERT() equals zero. */
#define configASSERT( x ) \
  do { \
    if( ( x ) == 0 ) vAssertCalled( __FILE__, __FUNCTION__, __LINE__ ); \
  } while (0)

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <timers.h>
#include <Wire.h>
#include "board.h"
#include "tasks_config.h"

struct WeatherReading {
  float fTempC;
  float fRh;
  float fLight;
  float fAltitudeM;
};

struct CarReading {
  uint32_t uRpm;
};

extern QueueHandle_t xCanQueue;
extern SemaphoreHandle_t xSerialLock;
extern SemaphoreHandle_t xSpiLock;
extern SemaphoreHandle_t xI2cLock;
extern SemaphoreHandle_t xWeatherReadingLock;
extern SemaphoreHandle_t xCarReadingLock;
extern WeatherReading xLastWeatherReading;
extern CarReading xLastCarReading;

template<class T>
inline static Print& operator <<(Print &obj, T arg)
{
  obj.print(arg); return obj;
}

inline static void prvDelay(unsigned long milliseconds)
{
  vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

inline static void prvDelayMicroseconds(unsigned int microseconds)
{
  vTaskDelay(pdMS_TO_TICKS(microseconds / 1000));
}

inline static unsigned long prvMillis(void)
{
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void vCoreInit(void);
void vCanInit(void);
void vWeatherInit(void);
void vShellInit(void);

#endif // __COMMON_H__
