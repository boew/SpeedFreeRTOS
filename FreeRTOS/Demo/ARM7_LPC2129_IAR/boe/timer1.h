#include "queue.h"
#include "task.h"
void setupTimer1( void );
void DelayResolution100us(uint32_t Delay);

typedef struct timeStoreElement_s {
  unsigned ManualPaddding;
  unsigned REFE;
  unsigned minuteCount;
  unsigned captureCount;
} timeStoreElement_t;

#define timer1_speedINVALID_TSE_INITIALIZER {0,0,-1,0}

extern QueueHandle_t timeStore;

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate);

/* count to 0xD2EFFFFF = 3538943999 to get 60 s (configCPU_CLOCK_HZ 58982400 is also pclk) */
#define timer1_TICKS_PER_MINUTE 0xD2F00000
#define timer1_MATCH_FOR_MINUTE (timer1_TICKS_PER_MINUTE - 1)
