#ifndef __TASKS_CONFIG_H__
#define __TASKS_CONFIG_H__

#define STATUS_LOG_TASK_NAME "StatusLog"
#define CAN_SNIFF_TASK_NAME "CanSniff"
#define WEATHER_TASK_NAME "WStation"
#define DISPLAY_TASK_NAME "Display"

#define CAN_SNIFF_TASK_PERIOD_TICKS pdMS_TO_TICKS(250)
#define WEATHER_TASK_PERIOD_TICKS pdMS_TO_TICKS(1000)
#define STATUS_LOG_TASK_PERIOD_TICKS pdMS_TO_TICKS(1000)

#define STATUS_LOG_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define DISPLAY_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define WEATHER_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CAN_SNIFF_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

#define STATUS_LOG_STACK_SIZE 512
#define CAN_SNIFF_STACK_SIZE 512
#define WEATHER_STACK_SIZE 512
#define DISPLAY_STACK_SIZE 512

#define CAN_QUEUE_LENGTH 10

void prvTaskStatusLog(void *pvParameters);
void prvTaskCanSniff(void *pvParameters);
void prvTaskWeather(void *pvParameters);
void prvTaskDisplay(void *pvParameters);

#endif // __TASKS_CONFIG_H__
