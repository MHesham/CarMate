#include "common.h"
#include <mcp2515.h>

SemaphoreHandle_t xCarReadingLock = NULL;
QueueHandle_t xCanQueue = NULL;
CarReading xLastCarReading;

void vCanInit(void)
{
  memset(&xLastCarReading, 0x0, sizeof(xLastWeatherReading));

  xCanQueue = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(can_frame));
  configASSERT(xCanQueue);

  xCarReadingLock = xSemaphoreCreateMutex();
  configASSERT(xCarReadingLock);
  xSemaphoreGive(xCarReadingLock);

/*
  xTaskCreate(prvTaskCanSniff,
              CAN_SNIFF_TASK_NAME,
              CAN_SNIFF_STACK_SIZE,
              NULL,
              CAN_SNIFF_TASK_PRIORITY,
              NULL);
*/
  xSemaphoreTake(xSerialLock, portMAX_DELAY);
  Serial << "CAN initialized\n";
  xSemaphoreGive(xSerialLock);
}

void prvTaskCanSniff(void *pvParameters)
{
  can_frame xCanMsg;
  MCP2515 xCanBus(boardCAN_MCP2515_CS_PIN, prvDelay, prvMillis);

  xSemaphoreTake(xSpiLock, portMAX_DELAY);
  xCanBus.reset();
  xCanBus.setBitrate(CAN_500KBPS);
  xCanBus.setNormalMode();
  xSemaphoreGive(xSpiLock);

  TickType_t xNextWakeTime = xTaskGetTickCount();

  for (;;) {
    xSemaphoreTake(xSpiLock, portMAX_DELAY);
    MCP2515::ERROR ret = xCanBus.readMessage(&xCanMsg);
    xSemaphoreGive(xSpiLock);

    if (ret == MCP2515::ERROR_OK) {
      xQueueSend(xCanQueue, &xCanMsg, portMAX_DELAY);
    };

    vTaskDelayUntil(&xNextWakeTime, CAN_SNIFF_TASK_PERIOD_TICKS);
  }
}
