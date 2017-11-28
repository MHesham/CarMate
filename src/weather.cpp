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

void prvTaskWeather(void *pvParameters);

void vWeatherInit(void)
{
  memset(&xLastWeatherReading, 0x0, sizeof(xLastWeatherReading));

  xTaskCreate(prvTaskWeather,
              WEATHER_TASK_NAME,
              WEATHER_STACK_SIZE,
              NULL,
              WEATHER_TASK_PRIORITY,
              NULL);

  vPrintf_P(PSTR("Weather started\n"));
}

void prvReadWeather(WeatherReading *pxReading)
{
  float fOperatingVoltage = analogRead(boardREFERENCE_3V3_PIN);
  float fLightSensor = analogRead(boardLIGHT_SENSOR_PIN);
  fOperatingVoltage = 3.3 / fOperatingVoltage; //The reference voltage is 3.3V
  pxReading->fLight = fOperatingVoltage * fLightSensor;

  portYIELD();

  float fTemp1C;
  {
    LockGuard lk(xI2cLock);
    fTemp1C = xWeather.readTemperature();
    pxReading->fRh = xWeather.readHumidity();
  }

  portYIELD();

  float fTemp2C;
  {
    LockGuard lk(xI2cLock);
    fTemp2C = xPressure.readTemp();
    pxReading->fAltitudeM = xPressure.readAltitude();
    /* The actual temp is the average between the 2 on-board temp sensors */
  }

  pxReading->fTempC = (fTemp1C + fTemp2C) / 2.0;
}

void prvTaskWeather(void *pvParameters)
{
  WeatherReading xCurrReading;

  pinMode(boardREFERENCE_3V3_PIN, INPUT);
  pinMode(boardLIGHT_SENSOR_PIN, INPUT);
  xSemaphoreTake(xI2cLock, portMAX_DELAY);
  xWeather.begin(Wire);
  xPressure.begin();
  /* Measure altitude above sea level in meters */
  xPressure.setModeAltimeter();
  /* According to the datasheet, 7 maps to 512ms sensor sampling periods */
  xPressure.setOversampleRate(7);
  /* Enable all three pressure and temp event flags */
  xPressure.enableEventFlags();
  xSemaphoreGive(xI2cLock);
  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    prvReadWeather(&xCurrReading);

    xSemaphoreTake(xWeatherReadingLock, portMAX_DELAY);
    xLastWeatherReading = xCurrReading;
    xSemaphoreGive(xWeatherReadingLock);

    vTaskDelayUntil(&xNextWakeTime, WEATHER_TASK_PERIOD_TICKS);
  }
}
