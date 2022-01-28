#include "drv_hd44780.h"
#include "drv_hd44780_cnfg.h"
#include "queue.h"

typedef struct lcdToShow_s {
  HD44780_XY_DEF x; 
  HD44780_XY_DEF y; 
  HD44780_STRING_DEF DataStr[HD44780_HORIZONTAL_SIZE + 1]; 
} lcdToShow_t;

extern QueueHandle_t LCDQ;

void vAltStartLCDTask( UBaseType_t uxPriority);