/* Standard includes. */
#include <stdlib.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"            
#define LIGHT_ON        1
#define LIGHT_OFF       0

#define LIGHT_SLOW      1
#define LIGHT_FAST      0

#define LIGHT_OFF_VALUE 0
#define LIGHT_ON_VALUE  0x3FF                
static void prvLightInit(void);
void prvLightCntr (uint8_t Slow, uint8_t On);
static portTASK_FUNCTION_PROTO( vLCDTask, pvParameters );

void vAltStartLCDTask( UBaseType_t uxPriority)
{
    xTaskCreate( vLCDTask, "LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );  
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
  HD44780_XY_DEF lcdx=1;
  HD44780_XY_DEF lcdy=1;
  HD44780_STRING_DEF txt[]="A Text";
  
	/* Just to stop compiler warnings. */
	( void ) pvParameters;
    HD44780_PowerUpInit();
    prvLightInit();
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