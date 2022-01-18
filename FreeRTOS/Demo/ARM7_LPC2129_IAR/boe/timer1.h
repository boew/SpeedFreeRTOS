#include "queue.h"
#include "task.h"
void setupTimer1( void );

typedef struct timeStoreElement_s {
  unsigned minuteCount;
  unsigned captureCount;
} timeStoreElement_t;

extern QueueHandle_t timeStore;

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate, UBaseType_t uxLED );