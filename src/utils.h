#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino_FreeRTOS.h>

template<class T>
inline static Print& operator <<(Print &obj, T arg)
{
  obj.print(arg); return obj;
}

inline static void vDelay(unsigned long milliseconds)
{
  vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

inline static void vDelayMicroseconds(unsigned int microseconds)
{
  vTaskDelay(pdMS_TO_TICKS(microseconds / 1000));
}

inline static unsigned long vMillis(void)
{
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void vPrintf_P(const char* format, ...);

#endif // __COMMON_H__
