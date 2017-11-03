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
  } while (0);

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

template<class T>
inline Print& operator <<(Print &obj, T arg)
{
  obj.print(arg); return obj;
}

#endif // __COMMON_H__
