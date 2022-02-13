#include "queue.h"
#include "task.h"
void setupTimer1( void );
void DelayResolution100us(uint32_t Delay);

typedef struct timeStoreElement_s {
  uint32_t ManualPaddding;
  uint32_t REFE;
  uint32_t minuteCount;
  uint32_t captureCount;
} timeStoreElement_t;

#define timer1_speedINVALID_TSE_MINUTE_COUNT ((1 << 30) - 1)

extern QueueHandle_t timeStore;

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate);

/* count to 0xD2EFFFFF = 3538943999 to get 60 s (configCPU_CLOCK_HZ 58982400 is also pclk) */
#define timer1_TICKS_PER_MINUTE 0xD2F00000
#define timer1_MATCH_FOR_MINUTE (timer1_TICKS_PER_MINUTE - 1)

/* 
 * Time for one rev in seconds is tick_diff / tick rate  
 * As of 2022-02-04 tick rate = system clock = 58982400
 * For stationary bike, expect time for one rev in the vicinity of one second  
 * Thus ignoring tick_diff <= 58982 (more than 1000 revs per second) to
 * get rid of fidgetspinner glitches,
 */

#define TOO_SMALL_TICK_DIFF 5898240 //10 revs, was 1000 58982
