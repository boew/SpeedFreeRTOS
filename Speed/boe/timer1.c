/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "timer1.h"
#include <intrinsics.h>

__arm void vTimer1ISR(void);

/* Constants to set up time store. */
//#define timeStoreLength 1024
#define timeStoreLength 128

static volatile unsigned timer1MinuteCount=0;
static volatile unsigned timer1EventCount=0;
static volatile unsigned timer1ShortEventCount=0;
static volatile unsigned timer1OKCaptures=0;

static QueueHandle_t initTimeStore(void);
QueueHandle_t timeStore;

static volatile int DelayResolution100usActive = 0;
//#define BOEDBG100us 1
#if BOEDBG100us 
volatile uint32_t huh=0;
volatile uint32_t hah=0;
volatile uint32_t heh=0;
volatile uint32_t Que;  
#endif 
void DelayResolution100us(uint32_t Delay)
{
  /* 
	 Want delay of 100 us  = 100 * 1e-6 s = 1e-4 s <=> 1e4 Hz
	 Have 58 982 400 Hz clock (configCPU_CLOCK_HZ 58982400 is also pclk)
	 58982400 / 1e4 = 5898.2400 
	 ==> Count 5898 ticks.
  */
  volatile uint32_t DRstart;
  volatile uint32_t DRstop;
  portENTER_CRITICAL();
  DRstart = T1TC;
  DRstop = DRstart + Delay * 5898;
  DelayResolution100usActive = 1;
  // Set match register 1
  T1MR1 = DRstop % timer1_TICKS_PER_MINUTE;
  T1IR_bit.MR1INT = 1; // Clear interrupt flag
  T1MCR_bit.MR1INT = 1; // Enable interrupt
  portEXIT_CRITICAL();  
  while(DelayResolution100usActive)
  {
#if BOEDBG100us 
    huh++;
    if (((Que=T1TC) < DRstart) || (DRstop < Que))
    {
      hah++;
      if (DelayResolution100usActive) 
      { 
        DelayResolution100usActive = 0; 
        heh++;
      }
    }
#else
    __no_operation();
#endif
  }
}

/*-----------------------------------------------------------*/

__arm void vTimer1ISR(void)
{
  static int servedOK=0;
  static uint32_t prvPreviousCapture = 0;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  timeStoreElement_t prvTse;
  if (T1IR_bit.MR0INT)
	{							/* MatchRegister - one minute. */
	  timer1MinuteCount++;
	  T1IR_bit.MR0INT = 1;		/* Clear match interrupt flag */
      servedOK |= 0x01;
	}
  if (T1IR_bit.MR1INT)
	{							/* MatchRegister for DelayResolution100us(). */
	  T1IR_bit.MR1INT = 1;		/* Clear match interrupt flag */
      T1MCR_bit.MR1INT = 0;     /* Disable match interrupt */
	  DelayResolution100usActive = 0;
      servedOK |= 0x02;	
    }
  if (T1IR_bit.CR2INT)
	{							/* Capture - one wheel rotaion  */
	  prvTse.REFE = IO0PIN;
      prvTse.captureCount = T1CR2;
	  prvTse.minuteCount = timer1MinuteCount;
      if ((prvTse.captureCount - prvPreviousCapture) > TOO_SMALL_TICK_DIFF)
		{
		  xQueueSendFromISR( timeStore, &prvTse, &xHigherPriorityTaskWoken);
		  prvPreviousCapture = prvTse.captureCount;
		  timer1OKCaptures++;
		}
      else
      {
        timer1ShortEventCount++; //__no_operation();
      }
	  T1IR_bit.CR2INT = 1;		/* Clear capture interrupt flag */
      servedOK |= 0x04;
	}
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  if  (servedOK)
  {
    VICVectAddr = 0; 				/* Update VIC priority hardware */
    servedOK = 0;
  } 
  else
  {
    while (1) { __no_operation(); }
  }
}

/*-----------------------------------------------------------*/

void setupTimer1( void )
{
  extern void ( vTimer1ISREntry) ( void );  
  portENTER_CRITICAL();
  //T1IR  
  T1TCR_bit.CE=0;
  T1TCR_bit.CR=1;
  //T1TC  
  T1PR = 0;
  //T1PC  
  T1MCR_bit.MR0INT = 1; 		/* Minute count */
  T1MCR_bit.MR0RES = 1; 
  T1MCR_bit.MR1INT = 0; 		/* DelayResolution100us */
  T1MCR_bit.MR1RES = 0; 
  T1MR0 = timer1_MATCH_FOR_MINUTE;	/* count to 0xD2EFFFFF = 3538943999 to get 60 s (configCPU_CLOCK_HZ 58982400 is also pclk) */
  //T1MR1 
  //T1MR2 
  //T1MR3 
  //T1CCR_bit.CAP2RE = 1; //Expect less bounce w/o rising edge (?)
  T1CCR_bit.CAP2FE = 1;
  T1CCR_bit.CAP2INT = 1;
  
  PINSEL1_bit.P0_17 = 1; // Timer 1 Capture 2
  //T1CR0 
  //T1CR1 
  //T1CR2 
  //T1CR3 
  //T1EMR 
  T1CTCR_bit.CTM = 0;

  VICVectAddr2 = (uint32_t) vTimer1ISREntry;
  VICVectCntl2_bit.NUMBER = VIC_TIMER1;
  VICVectCntl2_bit.ENABLED = 1;
  VICIntSelect_bit.INT5 = 0; // Redundant - is IRQ by default
  VICIntEnable_bit.INT5 = 1; // VIC_TIMER1
  
  T1IR_bit.MR0INT = 1;
  T1TCR_bit.CR=0;
  T1TCR_bit.CE=1;				/* Start counting */
  portEXIT_CRITICAL();
  /* TBD: minimal portE/E_CRITICAL range ? */
  timeStore = initTimeStore();
}

static QueueHandle_t initTimeStore()
{
  QueueHandle_t xQueue;
  xQueue = xQueueCreate( timeStoreLength, ( UBaseType_t ) sizeof( timeStoreElement_t ) );
  vQueueAddToRegistry( xQueue, "time_el_Q" );
  return (xQueue);
}

