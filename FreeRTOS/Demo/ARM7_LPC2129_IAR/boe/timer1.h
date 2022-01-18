void setupTimer1( void );

typedef struct timeStoreElement_s {
  unsigned minuteCount;
  unsigned capturecount;
} timeStoreElement_t;

QueueHandle_t timeStore;

