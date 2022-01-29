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
void prvLightCntr (uint8_t Slow, uint8_t On);
static portTASK_FUNCTION_PROTO( vLCDTask, pvParameters );

static lcdToShow_t prvlcdToShow = {1, 1, "0123456789ABCDEF"}; 

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
  TickType_t xTimeToWait;
  BaseType_t retval;
  xTimeToBlock = portMAX_DELAY; // ( TickType_t ) 0x32 ; 
  xTimeToWait =  ( TickType_t ) 0x200; //0x32 ; 
  /* Just to stop compiler warnings. */
  ( void ) pvParameters;
  HD44780_PowerUpInit();
  prvLightInit();
  xQueueSend(LCDQ, &prvlcdToShow, xTimeToBlock);
  //HD44780_StrShow(1, 1, "abcdefghijklmnop");
  //HD44780_StrShow(1, 2, "qrstuvwxyzABCDEF");
	for( ;; )
	{
#if 0      
      HD44780_StrShow(prvlcdToShow.x, prvlcdToShow.y, prvlcdToShow.DataStr);
      vTaskDelay( xTimeToWait );
      counter_LCD++;
      prvlcdToShow.DataStr[0] += 1; 
      if (prvlcdToShow.DataStr[0]  > 'Z')
      {
        prvlcdToShow.DataStr[0] = 'A';
        prvlcdToShow.DataStr[1] += 1;
        if (prvlcdToShow.DataStr[1]  > 'Z')
        { 
          prvlcdToShow.DataStr[1] = 'A';
        }
      }
#else
      retval = xQueueReceive(LCDQ, (void*) &prvlcdToShow, xTimeToBlock); // xTimeToWait xTimeToBlock);
      HD44780_StrShow(prvlcdToShow.x, prvlcdToShow.y, prvlcdToShow.DataStr);
#endif       
	}
} /*lint !e715 !e818 pvParameters is required for a task function even if it is not referenced. */

/*-----------------------------------------------------------*/

static QueueHandle_t initLCDQ(void)
{
  QueueHandle_t xQueue;
  xQueue = xQueueCreate( LCDQLength, ( UBaseType_t ) sizeof( lcdToShow_t ) );
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

/*************************************************************************
 * Function Name: LightCntr
 * Parameters: LPC_BOOL Slow,
               LPC_BOOL On
 * Return: none
 * Description: Light control
 *
 *************************************************************************/
void prvLightCntr (uint8_t Slow, uint8_t On)
{
  if(prvLightState != On)
  {
    if(Slow == LIGHT_SLOW)
    {
      if(On == LIGHT_ON)
      {
        if(DACR_bit.VALUE < LIGHT_ON_VALUE-0xF)
        {
          DACR_bit.VALUE += 0xF;
        }
        else
        {
          prvLightState = On;
        }
      }
      else
      {
        if(DACR_bit.VALUE > 0xF)
        {
          DACR_bit.VALUE -= 0xF;
        }
        else
        {
          prvLightState = On;
        }
      }
    }
    else
    {
      prvLightState = On;
      DACR_bit.VALUE = (On == LIGHT_ON)? LIGHT_ON_VALUE: LIGHT_OFF_VALUE;
    }
  }
}