#include "common.h"
#include <mcp2515.h>
#include "tasks_config.h"
#include "board_config.h"

QueueHandle_t xCanQueue = NULL;
SemaphoreHandle_t xSerialLock = NULL;
SemaphoreHandle_t xSpiLock = NULL;
SemaphoreHandle_t xI2cLock = NULL;
SemaphoreHandle_t xWeatherReadingLock = NULL;
SemaphoreHandle_t xCarReadingLock = NULL;

struct WeatherReading {
  float fTempC;
  float fRh;
  float fLight;
};

struct CarReading {
  uint32_t uRpm;
};

WeatherReading xLastWeatherReading;
CarReading xLastCarReading;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO,
      // MICRO, YUN, and other 32u4 based boards.
  }

  digitalWrite(STATUS_LED1, LOW);
  pinMode(STATUS_LED1, OUTPUT);

  digitalWrite(STATUS_ERROR_LED, LOW);
  pinMode(STATUS_ERROR_LED, OUTPUT);

  memset(&xLastWeatherReading, 0x0, sizeof(xLastWeatherReading));
  memset(&xLastCarReading, 0x0, sizeof(xLastWeatherReading));

  Serial << "CarMate RTOS is starting...";

  xSerialLock = xSemaphoreCreateMutex();
  configASSERT(xSerialLock);
  xSemaphoreGive(xSerialLock);

  xSpiLock = xSemaphoreCreateMutex();
  configASSERT(xSpiLock);
  xSemaphoreGive(xSpiLock);

  xI2cLock = xSemaphoreCreateMutex();
  configASSERT(xI2cLock);
  xSemaphoreGive(xI2cLock);

  xWeatherReadingLock = xSemaphoreCreateMutex();
  configASSERT(xWeatherReadingLock);
  xSemaphoreGive(xWeatherReadingLock);

  xCarReadingLock = xSemaphoreCreateMutex();
  configASSERT(xCarReadingLock);
  xSemaphoreGive(xCarReadingLock);

  xCanQueue = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(can_frame));
  configASSERT(xCanQueue);

  xTaskCreate(prvTaskStatusLog,
              STATUS_LOG_TASK_NAME,
              STATUS_LOG_STACK_SIZE,
              NULL,
              STATUS_LOG_TASK_PRIORITY,
              NULL);
/*
  xTaskCreate(prvTaskDisplay,
              DISPLAY_TASK_NAME,
              DISPLAY_STACK_SIZE,
              NULL,
              DISPLAY_TASK_PRIORITY,
              NULL);
*/
/*
  xTaskCreate(prvTaskCanSniff,
              CAN_SNIFF_TASK_NAME,
              CAN_SNIFF_STACK_SIZE,
              NULL,
              CAN_SNIFF_TASK_PRIORITY,
              NULL);
*/
  xTaskCreate(prvTaskWeather,
              WEATHER_TASK_NAME,
              WEATHER_STACK_SIZE,
              NULL,
              WEATHER_TASK_PRIORITY,
              NULL);
  // Now the task scheduler, which takes over control of scheduling individual
  // tasks, is automatically started.
}

void loop() {
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
//Returns the voltage of the light sensor based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float prvReadLightLevel()
{
  float fOperatingVoltage = analogRead(REFERENCE_3V3_PIN);
  float fLightSensor = analogRead(LIGHT_SENSOR_PIN);

  fOperatingVoltage = 3.3 / fOperatingVoltage; //The reference voltage is 3.3V
  fLightSensor = fOperatingVoltage * fLightSensor;
  return fLightSensor;
}

float prvReadHumidity()
{
  return 0.0f;
}

float prvReadTempC()
{
  return 0.0f;
}

void prvTaskWeather(void *pvParameters)
{
  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << WEATHER_TASK_NAME " is starting\n";
  xSemaphoreGive(xSerialLock);
  WeatherReading xCurrReading;

  pinMode(REFERENCE_3V3_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    xCurrReading.fLight = prvReadLightLevel();
    xCurrReading.fTempC = prvReadTempC();
    xCurrReading.fRh = prvReadHumidity();

    xSemaphoreTake(xWeatherReadingLock, portMAX_DELAY);
    xLastWeatherReading = xCurrReading;
    xSemaphoreGive(xWeatherReadingLock);

    vTaskDelayUntil(&xNextWakeTime, WEATHER_TASK_PERIOD_TICKS);
  }
}

void prvTaskDisplay(void *pvParameters)
{
  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << DISPLAY_TASK_NAME " is starting\n";
  xSemaphoreGive(xSerialLock);

  for (;;) {
  }
}

void prvTaskStatusLog(void *pvParameters)
{
  CarReading xCurrCarReading;
  WeatherReading xCurrWeatherReading;

  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << STATUS_LOG_TASK_NAME " is starting\n";
  xSemaphoreGive(xSerialLock);

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {

    xSemaphoreTake(xCarReadingLock, portMAX_DELAY);
    xCurrCarReading = xLastCarReading;
    xSemaphoreGive(xCarReadingLock);

    xSemaphoreTake(xWeatherReadingLock, portMAX_DELAY);
    xCurrWeatherReading = xLastWeatherReading;
    xSemaphoreGive(xWeatherReadingLock);

    xSemaphoreTake(xSerialLock, portMAX_DELAY);
    Serial << "Temp:" << xCurrWeatherReading.fTempC
           << " RH:" << xCurrWeatherReading.fRh
           << " Light:" << xCurrWeatherReading.fLight
           << " | RPM:" << xCurrCarReading.uRpm
           << ".\n";
    xSemaphoreGive(xSerialLock);

    vTaskDelayUntil(&xNextWakeTime, STATUS_LOG_TASK_PERIOD_TICKS);
  }
}

void prvDelay(unsigned long milliseconds)
{
  vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

unsigned long prvMillis(void)
{
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void prvTaskCanSniff(void *pvParameters)
{
  can_frame xCanMsg;
  MCP2515 xCanBus(CAN_MCP2515_CS_PIN, prvDelay, prvMillis);
  bool pinState = false;
  const uint8_t uStatusLed = STATUS_LED1;

  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << CAN_SNIFF_TASK_NAME " is starting\n";
  xSemaphoreGive(xSerialLock);

  xSemaphoreTake(xSpiLock, portMAX_DELAY);
  xCanBus.reset();
  xCanBus.setBitrate(CAN_500KBPS);
  xCanBus.setNormalMode();
  xSemaphoreGive(xSpiLock);

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    digitalWrite(uStatusLed, (pinState ? HIGH : LOW));
    pinState = !pinState;

    xSemaphoreTake(xSpiLock, portMAX_DELAY);
    MCP2515::ERROR ret = xCanBus.readMessage(&xCanMsg);
    xSemaphoreGive(xSpiLock);

    if (ret == MCP2515::ERROR_OK) {
      xQueueSend(xCanQueue, &xCanMsg, portMAX_DELAY);
    };

    vTaskDelayUntil(&xNextWakeTime, CAN_SNIFF_TASK_PERIOD_TICKS);
  }
}
