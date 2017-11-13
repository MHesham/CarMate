#include "common.h"
#include <LiquidCrystal_I2C.h>

char cTaskList[512];

void vShellInit(void)
{
  xTaskCreate(prvTaskStatusLog,
              STATUS_LOG_TASK_NAME,
              STATUS_LOG_STACK_SIZE,
              NULL,
              STATUS_LOG_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvTaskDisplay,
              DISPLAY_TASK_NAME,
              DISPLAY_STACK_SIZE,
              NULL,
              DISPLAY_TASK_PRIORITY,
              NULL);

  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << "Shell initialized\n";
  xSemaphoreGive(xSerialLock);
}

void prvTaskDisplay(void *pvParameters)
{
  WeatherReading xCurrWeatherReading;
  // set the LCD address to 0x27 for a 16 chars and 2 line display
  LiquidCrystal_I2C lcd(0x27, 16, 2, LCD_5x8DOTS, prvDelay, prvDelayMicroseconds);

  xSemaphoreTake(xI2cLock, portMAX_DELAY);
  lcd.begin();
  lcd.backlight();
  xSemaphoreGive(xI2cLock);

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    xSemaphoreTake(xWeatherReadingLock, portMAX_DELAY);
    xCurrWeatherReading = xLastWeatherReading;
    xSemaphoreGive(xWeatherReadingLock);

    xSemaphoreTake(xI2cLock, portMAX_DELAY);
    xSemaphoreTake(xSerialLock, portMAX_DELAY);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd << xCurrWeatherReading.fTempC << "C "
        << xCurrWeatherReading.fRh << "RH";
    lcd.setCursor(0, 1);
    lcd << xCurrWeatherReading.fAltitudeM << "M "
        << xCurrWeatherReading.fLight << "V";
    xSemaphoreGive(xSerialLock);
    xSemaphoreGive(xI2cLock);

    vTaskDelayUntil(&xNextWakeTime, DISPLAY_TASK_PERIOD_TICKS);
  }
}

void prvTaskStatusLog(void *pvParameters)
{
  CarReading xCurrCarReading;
  WeatherReading xCurrWeatherReading;

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    vTaskGetRunTimeStats(cTaskList);
    xSemaphoreTake(xCarReadingLock, portMAX_DELAY);
    xCurrCarReading = xLastCarReading;
    xSemaphoreGive(xCarReadingLock);

    xSemaphoreTake(xWeatherReadingLock, portMAX_DELAY);
    xCurrWeatherReading = xLastWeatherReading;
    xSemaphoreGive(xWeatherReadingLock);

    xSemaphoreTake(xSerialLock, portMAX_DELAY);
    Serial << vGetTimerForRunTimeStats()
           << " uTemp(C):" << xCurrWeatherReading.fTempC
           << " RH:" << xCurrWeatherReading.fRh
           << " Alt(m):" << xCurrWeatherReading.fAltitudeM
           << " Light(V):" << xCurrWeatherReading.fLight
           << " | RPM:" << xCurrCarReading.uRpm << "\n"
           << cTaskList << "\n";
    xSemaphoreGive(xSerialLock);
    vTaskDelayUntil(&xNextWakeTime, STATUS_LOG_TASK_PERIOD_TICKS);
  }
}
