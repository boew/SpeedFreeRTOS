#ifndef LCD_H__ 
#define LCD_H__ 
#include "drv_hd44780.h"
#include "drv_hd44780_cnfg.h"
#include "queue.h"

typedef HD44780_STRING_DEF lcdLine_t[HD44780_HORIZONTAL_SIZE + 1]; 
typedef struct lcdToShow_s {
  HD44780_XY_DEF x; 
  HD44780_XY_DEF y; 
   lcdLine_t DataStr;
} lcdToShow_t;

extern QueueHandle_t LCDQ;

void vStartLCDTask( UBaseType_t uxPriority);
#endif