/* Standard includes. */
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "lcd.h"            

#define LIGHT_ON        1
#define LIGHT_OFF       0

#define LIGHT_SLOW      1
#define LIGHT_FAST      0

#define LIGHT_OFF_VALUE 0
#define LIGHT_ON_VALUE  0x3FF                

#define LCDQLength      10
static void prvLightInit(void);
static portTASK_FUNCTION_PROTO( vLCDTask, pvParameters );

static lcd2Show_t prvlcd2Show = {{1, 1, "0123456789ABCDEF"}, {1, 2, "GHIJKLMNOPQRSTUV"}}; 

static QueueHandle_t initLCDQ(void);
QueueHandle_t LCDQ;
void vStartLCDTask( UBaseType_t uxPriority)
{
    xTaskCreate( vLCDTask, "LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );  
    LCDQ = initLCDQ();
}
uint8_t prvLightState;

static portTASK_FUNCTION( vLCDTask, pvParameters )
{
  volatile uint32_t counter_LCD;
  TickType_t xTimeToBlock;
  xTimeToBlock = portMAX_DELAY; 
  /* Just to stop compiler warnings. */
  ( void ) pvParameters;
  HD44780_PowerUpInit();
  prvLightInit();
  CGRAM_INIT();
  xQueueSend(LCDQ, &prvlcd2Show, xTimeToBlock);
	for( ;; )
	{
      xQueueReceive(LCDQ, (void*) &prvlcd2Show, xTimeToBlock); 
	  HD44780_ClearDisplay();
      HD44780_StrShow(prvlcd2Show.l1.x, prvlcd2Show.l1.y, prvlcd2Show.l1.DataStr);
      HD44780_StrShow(prvlcd2Show.l2.x, prvlcd2Show.l2.y, prvlcd2Show.l2.DataStr);      
	}
} /*lint !e715 !e818 pvParameters is required for a task function even if it is not referenced. */

/*-----------------------------------------------------------*/

static QueueHandle_t initLCDQ(void)
{
  QueueHandle_t xQueue;
  xQueue = xQueueCreate( LCDQLength, ( UBaseType_t ) sizeof( lcd2Show_t ) );
  vQueueAddToRegistry( xQueue, "LCDQ" );
  return (xQueue);
}  

/*************************************************************************
 * Function Name: LightInit
 * Parameters: none
 * Return: none
 * Description: Init light control
 *
 *************************************************************************/
static void prvLightInit(void)
{
  /* Connect DAC output to pin P0_25 */
  PINSEL1_bit.P0_25 = 2;
  /* DAC Init */
  DACR_bit.BIAS = 0x1;
  /* Light Off */
  DACR_bit.VALUE = LIGHT_ON_VALUE; //LIGHT_OFF_VALUE;
  /* Init Current state of light */
  prvLightState = LIGHT_ON; // LIGHT_OFF;

}
