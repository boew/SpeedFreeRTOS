#include "queue.h"
#include "task.h"
void setupTimer1( void );

typedef struct timeStoreElement_s {
  unsigned minuteCount;
  unsigned capturecount;
} timeStoreElement_t;

extern QueueHandle_t timeStore;

