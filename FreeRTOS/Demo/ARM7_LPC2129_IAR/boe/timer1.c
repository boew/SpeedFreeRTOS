/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "timer1.h"

__arm void vTimer1ISR(void);

/* Constants to set up time store. */
//#define timeStoreLength 1024
#define timeStoreLength 256

static volatile unsigned timer1MinuteCount=0;
static volatile unsigned timer1EventCount=0;

static QueueHandle_t initTimeStore(void);
QueueHandle_t timeStore;

static volatile int DelayResolution100usActive = 0;
void DelayResolution100us(uint32_t Delay)
{
  /* 
	 Want delay of 100 us  = 100 * 1e-6 s = 1e-4 s <=> 1e4 Hz
	 Have 58 982 400 Hz clock (configCPU_CLOCK_HZ 58982400 is also pclk)
	 58982400 / 1e4 = 5898.2400 
	 ==> Count 5898 ticks.
  */
  uint32_t DRstart;
  uint32_t DRstop;
  uint32_t DR3;
  portENTER_CRITICAL();
  DRstart = T1TC;
  DRstop = DRstart + Delay * 5898;
  // Set match register 1
  T1MR1 = DRstop;
  T1MCR_bit.MR1INT = 1;
  DelayResolution100usActive = 1;
  DR3 = T1TC;
  portEXIT_CRITICAL();  
  while(DelayResolution100usActive);
}

/*-----------------------------------------------------------*/

__arm void vTimer1ISR(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  timeStoreElement_t tse;
  if (T1IR_bit.MR0INT)
	{							/* MatchRegister - one minute. */
	  timer1MinuteCount++;
	  T1IR_bit.MR0INT = 1;		/* Clear match interrupt flag */
	}
  if (T1IR_bit.MR1INT)
	{							/* MatchRegister for DelayResolution100us(). */
	  T1IR_bit.MR0INT = 1;		/* Clear match interrupt flag */
	  DelayResolution100usActive = 0;
	}
  if (T1IR_bit.CR2INT)
	{							/* Capture - one wheel rotaion  */
	  tse.REFE = IO0PIN;
      tse.captureCount = T1CR2;
	  tse.minuteCount = timer1MinuteCount;
	  xQueueSendFromISR( timeStore, &tse, &xHigherPriorityTaskWoken);
	  timer1EventCount++;
	  T1IR_bit.CR2INT = 1;		/* Clear capture interrupt flag */
	}
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  VICVectAddr = 0; 				/* Update VIC priority hardware */
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
  T1MR0 = 0xD2EFFFFF;	/* count to 0xD2EFFFFF = 3538943999 to get 60 s (configCPU_CLOCK_HZ 58982400 is also pclk) */
  //T1MR1 
  //T1MR2 
  //T1MR3 
  T1CCR_bit.CAP2RE = 1; //Expect less bounce w/o rising edge (?)
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

