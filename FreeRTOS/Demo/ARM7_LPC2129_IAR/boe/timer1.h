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

extern QueueHandle_t timeStore;

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate);
