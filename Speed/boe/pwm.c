/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "PWM.h"
#include <intrinsics.h>
__arm void vPWMISR(void);
static volatile int DelayResolution100usActive = 0;
#define BOEDBG100us 1
#if BOEDBG100us 
volatile uint32_t huh=0;
volatile uint32_t hah=0;
volatile uint32_t heh=0;
volatile uint32_t Que;  
#endif 
void DelayResolution100us(uint32_t Delay)
{
  portENTER_CRITICAL();
  DelayResolution100usActive = 1;
  //PWMMCR_bit.MR0INT = 1;        /* Enable interrupt */
  PWMMR0 = 5898 * Delay;				/* ###NOT prescaled to 100 us */
  //PWMMR0 = 10000 * Delay;				/* ###NOT prescaled to 100 us */
  PWMTCR_bit.CR=0;
  PWMTCR_bit.CE=1;				/* Start counting */
  portEXIT_CRITICAL();  
  while(DelayResolution100usActive)
  {
#if BOEDBG100us 
    huh++;
//    if ((Que=PWMTC) >= Delay )
      if ((Que=PWMTC) >= PWMMR0 )
    {
      hah++;
      if (DelayResolution100usActive) 
      { 
        DelayResolution100usActive = 0; 
        heh++;
        PWMTCR_bit.CE=0;				/* Stop counting */
        PWMTCR_bit.CR=1;                /* Reset count */
      }
    }
#else
    __no_operation();
#endif
  }
}

/*-----------------------------------------------------------*/
void setupPWM( void )
{
  /* 
	 Want delay of 100 us  = 100 * 1e-6 s = 1e-4 s <=> 1e4 Hz
	 Have 58 982 400 Hz clock (configCPU_CLOCK_HZ 58982400 is also pclk)
	 58982400 / 1e4 = 5898.2400 
	 ==> Prescaler Count = 5898-1 = 5897.
  */

  extern void ( vPWMISREntry) ( void );  
  portENTER_CRITICAL();
  PWMTCR_bit.CE=0;
  PWMTCR_bit.CR=1;
  PWMMCR_bit.MR0RES = 1; 
  PWMMCR_bit.MR0STOP = 1; 
  portEXIT_CRITICAL();
  /* TBD: minimal portE/E_CRITICAL range ? */
}
