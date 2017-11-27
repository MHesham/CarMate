#ifndef __CARMATE_H__
#define __CARMATE_H__

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

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
extern SemaphoreHandle_t xSpiLock;
extern SemaphoreHandle_t xI2cLock;
extern SemaphoreHandle_t xWeatherReadingLock;
extern SemaphoreHandle_t xCarReadingLock;
extern WeatherReading xLastWeatherReading;
extern CarReading xLastCarReading;
extern EventGroupHandle_t xSystemEvents;

#define carmateSYSTEM_UP_EVT_BIT 0x1

void vCoreInit(void);
void vCanInit(void);
void vWeatherInit(void);
void vShellInit(void);
void vSerialConsoleInit(void);
void vRegisterCLICommands(void);

#endif  // __CARMATE_H__