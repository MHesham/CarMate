#include <Arduino_FreeRTOS.h>
#include <SparkFunHTU21D.h>
#include <SparkFunMPL3115A2.h>
#include <lock.h>
#include "carmate.h"
#include "board.h"
#include "tasks_config.h"
#include "utils.h"

static HTU21D xWeather;
static MPL3115A2 xPressure;
WeatherReading xLastWeatherReading;

void prvHTU21DTask(void *pvParameters);
void prvMPL3115A2Task(void *pvParameters);
void prvLightTask(void *pvParameters);

void vWeatherInit(void) {
  memset(&xLastWeatherReading, 0x0, sizeof(xLastWeatherReading));

  xTaskCreate(prvHTU21DTask, tskcfgHTU21D_TASK_NAME,
              tskcfgHTU21D_TASK_STACK_SIZE, NULL, tskcfgHTU21D_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvMPL3115A2Task, tskcfgMPL3115A2_TASK_NAME,
              tskcfgMPL3115A2_TASK_STACK_SIZE, NULL,
              tskcfgMPL3115A2_TASK_PRIORITY, NULL);

  xTaskCreate(prvLightTask, tskcfgLIGHT_TASK_NAME, tskcfgLIGHT_TASK_STACK_SIZE,
              NULL, tskcfgLIGHT_TASK_PRIORITY, NULL);

  vPrintf_P(PSTR("Weather started\n"));
}

void prvReadWeather(WeatherReading *pxReading) {
  taskYIELD();

  float fTemp1C;
  {
    LockGuard lk(xI2cLock);
    fTemp1C = xWeather.readTemperature();
    pxReading->fRh = xWeather.readHumidity();
  }

  taskYIELD();

  float fTemp2C;
  {
    LockGuard lk(xI2cLock);
    fTemp2C = xPressure.readTemp();
    pxReading->fAltitudeM = xPressure.readAltitude();
    /* The actual temp is the average between the 2 on-board temp sensors */
  }
}

void prvHTU21DTask(void *pvParameters) {
  {
    LockGuard lk(xI2cLock);
    xWeather.begin(Wire);
  }

  TickType_t xNextWakeTime = xTaskGetTickCount();
  float fTemp1C;
  float fRh;

  for (;;) {
    {
      LockGuard lk(xI2cLock);
      fTemp1C = xWeather.readTemperature();
      fRh = xWeather.readHumidity();
    }

    {
      LockGuard lk(xWeatherReadingLock);
      xLastWeatherReading.fTemp1C = fTemp1C;
      xLastWeatherReading.fRh = fRh;
    }

    vTaskDelayUntil(&xNextWakeTime, tskcfgHTU21D_TASK_PERIOD_TICKS);
  }
}

void prvMPL3115A2Task(void *pvParameters) {
  {
    LockGuard lk(xI2cLock);
    xPressure.begin();
    /* Measure altitude above sea level in meters */
    xPressure.setModeAltimeter();
    /* According to the datasheet, 7 maps to 512ms sensor sampling periods */
    xPressure.setOversampleRate(7);
    /* Enable all three pressure and temp event flags */
    xPressure.enableEventFlags();
  }

  float fAltitudeM;
  float fTemp2C;

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    {
      LockGuard lk(xI2cLock);
      fTemp2C = xPressure.readTemp();
      fAltitudeM = xPressure.readAltitude();
    }

    {
      LockGuard lk(xWeatherReadingLock);
      xLastWeatherReading.fTemp2C = fTemp2C;
      xLastWeatherReading.fAltitudeM = fAltitudeM;
    }

    vTaskDelayUntil(&xNextWakeTime, tskcfgLIGHT_TASK_PERIOD_TICKS);
  }
}

void prvLightTask(void *pvParameters) {
  pinMode(boardREFERENCE_3V3_PIN, INPUT);
  pinMode(boardLIGHT_SENSOR_PIN, INPUT);
  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    {
      LockGuard lk(xWeatherReadingLock);
      float fOperatingVoltage = analogRead(boardREFERENCE_3V3_PIN);
      float fLightSensor = analogRead(boardLIGHT_SENSOR_PIN);
      fOperatingVoltage =
          3.3 / fOperatingVoltage;  // The reference voltage is 3.3V
      xLastWeatherReading.fLight = fOperatingVoltage * fLightSensor;
    }

    vTaskDelayUntil(&xNextWakeTime, tskcfgLIGHT_TASK_PERIOD_TICKS);
  }
}
