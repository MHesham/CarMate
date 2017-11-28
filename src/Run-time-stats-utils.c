#include <Arduino_FreeRTOS.h>

static unsigned long ulTimerTicks;

void vConfigureTimerForRunTimeStats(void) {
  ulTimerTicks = 0;

  /* Initialize Timer3 registers */
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  TIMSK3 = 0;

  /* Set compare match register to desired count
  (target period) = (timer resolution) * (# timer counts + 1) where
    (target period): 1 / configRUN_TIME_STATS_OVERSAMPLING_RATE_HZ
    (timer resolution):  1 / (configCPU_CLOCK_HZ / 1024)
    (# timer counts): the unknown Compare Match value to generate the desired
    target period.
    solving for (# timer counts)
    1 / configRUN_TIME_STATS_OVERSAMPLING_RATE_HZ = (# timer counts + 1) /
  (configCPU_CLOCK_HZ / 1024)
    (# timer counts + 1) = (configCPU_CLOCK_HZ / 1024) /
  configRUN_TIME_STATS_OVERSAMPLING_RATE_HZ
    (# timer counts) = (configCPU_CLOCK_HZ / (1024 *
  configRUN_TIME_STATS_OVERSAMPLING_RATE_HZ)) - 1
  */
  uint32_t ulCompareMatch = configCPU_CLOCK_HZ;
  ulCompareMatch /= (1024ul * configRUN_TIME_STATS_OVERSAMPLING_RATE_HZ);
  ulCompareMatch -= 1;

  /* Set high and low bits of the compare match */
  OCR3AL = (ulCompareMatch & (uint32_t)0xff);
  ulCompareMatch >>= 8;
  OCR3AH = (ulCompareMatch & (uint32_t)0xff);

  /* Set the Clear Timer on Compare Match (CTC) mode */
  TCCR3B |= _BV(WGM32);

  /* Set CS32 and CS30 bit so timer runs at clock speed 16MHz / 1024 = 15625Hz
  which equals to 0.000064s (64us)*/
  TCCR3B |= _BV(CS32) | _BV(CS30);

  /* Enable timer compare match interrupt */
  TIMSK3 |= _BV(OCIE3A);
}

unsigned long vGetTimerForRunTimeStats(void) { return ulTimerTicks; }

ISR(TIMER3_COMPA_vect) __attribute__((hot, flatten));
ISR(TIMER3_COMPA_vect) { ++ulTimerTicks; }