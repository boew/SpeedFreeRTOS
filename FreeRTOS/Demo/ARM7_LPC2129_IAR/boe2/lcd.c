/* Standard includes. */
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"            
                 
static portTASK_FUNCTION_PROTO( vLCDTask, pvParameters );

void vAltStartLCDTask( UBaseType_t uxPriority)
{
    xTaskCreate( vLCDTask, "LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );  
}

static portTASK_FUNCTION( vLCDTask, pvParameters )
{
  volatile uint32_t counter_LCD;
  TickType_t xTimeToBlock;
  TickType_t xTimeToWait;
  BaseType_t retval;
  xTimeToBlock = portMAX_DELAY; // ( TickType_t ) 0x32 ; 
  xTimeToWait =  ( TickType_t ) 0x32 ; 
  HD44780_XY_DEF lcdx=1;
  HD44780_XY_DEF lcdy=1;
  HD44780_STRING_DEF txt[]="A Text";
  
	/* Just to stop compiler warnings. */
	( void ) pvParameters;
    HD44780_PowerUpInit();
	for( ;; )
	{
      HD44780_StrShow(lcdx, lcdy, txt);
      vTaskDelay( xTimeToWait );
      counter_LCD++;
      txt[0] += 1; 
      if (txt[0]  > 'Z')
      {
        txt[0] = 'A';
        txt[1] += 1;
        if (txt[1]  > 'Z')
        { 
          txt[1] = 'A';
        }
      }  
	}
} /*lint !e715 !e818 pvParameters is required for a task function even if it is not referenced. */

/*-----------------------------------------------------------*/
