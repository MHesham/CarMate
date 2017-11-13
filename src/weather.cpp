#include "common.h"
#include <SparkFunHTU21D.h>
#include <SparkFunMPL3115A2.h>

SemaphoreHandle_t xWeatherReadingLock = NULL;
WeatherReading xLastWeatherReading;

HTU21D xWeather;
MPL3115A2 xPressure;

void vWeatherInit(void)
{
  memset(&xLastWeatherReading, 0x0, sizeof(xLastWeatherReading));

  xWeatherReadingLock = xSemaphoreCreateMutex();
  configASSERT(xWeatherReadingLock);
  xSemaphoreGive(xWeatherReadingLock);

  xTaskCreate(prvTaskWeather,
              WEATHER_TASK_NAME,
              WEATHER_STACK_SIZE,
              NULL,
              WEATHER_TASK_PRIORITY,
              NULL);

  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << "Weather initialized\n";
  xSemaphoreGive(xSerialLock);
}

void prvReadWeather(WeatherReading *pxReading)
{
  float fOperatingVoltage = analogRead(boardREFERENCE_3V3_PIN);
  float fLightSensor = analogRead(boardLIGHT_SENSOR_PIN);
  float fTemp1C, fTemp2C;

  fOperatingVoltage = 3.3 / fOperatingVoltage; //The reference voltage is 3.3V
  pxReading->fLight = fOperatingVoltage * fLightSensor;
  xSemaphoreTake(xI2cLock, portMAX_DELAY);
  fTemp1C = xWeather.readTemperature();
  pxReading->fRh = xWeather.readHumidity();
  fTemp2C = xPressure.readTemp();
  pxReading->fAltitudeM = xPressure.readAltitude();
  /* The actual temp is the average between the 2 on-board temp sensors */
  pxReading->fTempC = (fTemp1C + fTemp2C) / 2.0;
  xSemaphoreGive(xI2cLock);
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
