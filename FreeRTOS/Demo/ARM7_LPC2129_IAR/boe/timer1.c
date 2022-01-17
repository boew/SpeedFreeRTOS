/* Standard includes. */
#include <stdlib.h>
#include <intrinsics.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "timer1.h"
#include "queue.h"
/* Constants to set up timer 1 for speed sensing. */
#define tim1ENABLE_TIMER			( ( uint8_t )  0x01 )
#define tim1PRESCALE_VALUE						   0x00
#define portINTERRUPT_ON_MATCH		( ( uint32_t ) 0x01 )
#define portRESET_COUNT_ON_MATCH	( ( uint32_t ) 0x02 )


static __irq __arm void timer1ISR(void);

/* Constants to set up time store. */
#define timeStoreLength 1024

static volatile unsigned timer1MinuteCount=0;
typedef struct timeStoreElement_s {
  unsigned minuteCount;
  unsigned capturecount;
} timeStoreElement_t;

static void storeTimeForSpeed(unsigned minuteCount, unsigned timeCaptured);
static QueueHandle_t initTimeStore(void);
static volatile QueueHandle_t timeStore;

static __irq __arm void timer1ISR(void)
{
  if (T1IR_bit.MR0INT)
	{
	  timer1MinuteCount++;
	  T1IR_bit.MR0INT = 1;		/* Clear timer interrupt flag */
	}
  if (T1IR_bit.CR2INT)
	{
	  storeTimeForSpeed(timer1MinuteCount, T1CR2);
	  T1IR_bit.CR2INT = 1;		/* Clear timer interrupt flag */
	}
  VICVectAddr = 0; 				/* Update VIC priority hardware */
}

void setupTimer1( void )
{
  portENTER_CRITICAL();
  //T1IR  
  T1TCR_bit.CE=0;
  T1TCR_bit.CR=1;
  //T1TC  
  T1PR = 0;
  //T1PC  
  T1MCR_bit.MR0INT = 1;
  T1MCR_bit.MR0RES = 1; 
  T1MR0 = 0xD2EFFFFF;			/* count to 0xD2EFFFFF = 3538943999 to get 60 s (configCPU_CLOCK_HZ 58982400 is also pclk) */
  //T1MR1 
  //T1MR2 
  //T1MR3 
  T1CCR_bit.CAP2RE = 1;
  T1CCR_bit.CAP2FE = 1;
  T1CCR_bit.CAP2INT = 1;
  //T1CR0 
  //T1CR1 
  //T1CR2 
  //T1CR3 
  //T1EMR 
  T1CTCR_bit.CTM = 0;

  VICVectAddr2 = (unsigned long) timer1ISR;
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


#if 0
static void storeTimeForSpeed(unsigned minuteCount, unsigned timeCaptured)
{
  ;
}
static void initStore()
{
  ;
}
#else
static void storeTimeForSpeed(unsigned minuteCount, unsigned timeCaptured)
{
  timeStoreElement_t tse = {  minuteCount, timeCaptured };
  xQueueSend( timeStore, (void *) (&tse), (TickType_t) 0 );
}
static QueueHandle_t initTimeStore()
{
  return (xQueueCreate( timeStoreLength, ( UBaseType_t ) sizeof( timeStoreElement_t ) ));
}

#endif 

