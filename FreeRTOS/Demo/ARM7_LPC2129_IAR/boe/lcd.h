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

/* CGRAM */
#define CGRAM_m \001
#define CGRAM_p \002
#define CGRAM_r \003
#define CGRAM_s \004

#define CGRAM_INIT() \
{\
  HD44780_WrCGRAM("\x1F\x00\x1A\x15\x15\x11\x11\x00", 1);\
  HD44780_WrCGRAM("\x1F\x00\x1E\x11\x1E\x10\x10\x00", 2);\
  HD44780_WrCGRAM("\x1F\x00\x16\x19\x10\x10\x10\x00", 3);\
  HD44780_WrCGRAM("\x1F\x00\x0E\x10\x0E\x01\x1E\x00", 4);\
}

#define _xstr(a) #a
#define xstr(a) _xstr(a)

#endif
