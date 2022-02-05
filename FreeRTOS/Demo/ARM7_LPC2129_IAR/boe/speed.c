/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/* Demo program include files. */
#include "serial.h"
#include "comtest.h"
#include "queue.h"
#include "FreeRTOS.h"
#include "timer1.h"
#include "speed.h"

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME			( ( TickType_t ) 0xffff )

/* Handle to the com port used by Rx & Tx tasks. */
static xComPortHandle xPort = NULL;
#define timestoretestBUFFERSIZE 400
#define prvPRINTBUFFERSIZE 80
#define prvSIGNON "mC\tcC\r\n"
#define prvREV_TIMEOUT_MS 1000
/*-----------------------------------------------------------*/

static portTASK_FUNCTION_PROTO( vSpeedTask, pvParameters );
static lcdToShow_t prvSpeedToShowLine1 = {1,1, " ---            "};
static lcdToShow_t prvSpeedToShowLine2 = {1,2, " ===            "};
static void prvShowSpeed(void);
static void prvSpeedUpdate(timeStoreElement_t tse_buf);
static void prvSpeedLog(timeStoreElement_t tse_buf);
static void prvSpeedCalc(timeStoreElement_t tse_buf);
static void prvInitHistory(void);

void vStartSpeedTask( UBaseType_t uxPriority, uint32_t ulBaudRate)
{
  prvInitHistory();
  xPort = xSerialPortInitMinimal( ulBaudRate, timestoretestBUFFERSIZE );  
  xTaskCreate( vSpeedTask, "Speed", 2 * configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL ); 
  vSerialPutString(xPort, prvSIGNON, sizeof(prvSIGNON));
}

static signed char prvPrintBuffer[prvPRINTBUFFERSIZE]; 
static portTASK_FUNCTION( vSpeedTask, pvParameters )
{
  timeStoreElement_t tse_buf;
  TickType_t xTimeToWait =  ( TickType_t ) (prvREV_TIMEOUT_MS * portTICK_PERIOD_MS) ;
  /* Avoid compiler warnings -- pvParameters required even if not referenced. */
  ( void ) pvParameters;
  for( ;; )
  {
    switch (xQueueReceive(timeStore, (void*) &tse_buf, xTimeToWait))
    {
    case errQUEUE_EMPTY:
      snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), " Sensor timeout.");
      prvShowSpeed();
      break;
    case pdPASS:
      prvSpeedUpdate(tse_buf);
      break;
    default: 
      ;
    }
  }
} 

/*-----------------------------------------------------------*/
static void prvSpeedLog(timeStoreElement_t tse_buf)
{
  BaseType_t retval;
  retval = snprintf(&prvPrintBuffer, sizeof(prvPrintBuffer), "%03d\t0x%08X\t%01u\r\n",
                     tse_buf.minuteCount, tse_buf.captureCount, (tse_buf.REFE & (1 << 17)) >> 17 );
  vSerialPutString(xPort, prvPrintBuffer, prvPRINTBUFFERSIZE);
}

// (1 << N) -- circular buffer length is power of two
#define prvHMAX  (1 << 2)
static timeStoreElement_t prvHistory[prvHMAX];

static void prvInitHistory(void)
{
  int i;
  for(i=0; i < prvHMAX; i++)
  {
    prvHistory[i].minuteCount = timer1_speedINVALID_TSE_MINUTE_COUNT;
  }
}

static void prvSpeedCalc(timeStoreElement_t tse_buf)
{
  static uint32_t start = 0;
  static uint32_t missing = prvHMAX;
  
  uint32_t j;
  uint32_t h0;
  uint32_t h1;
  const uint64_t tick_Hz = configCPU_CLOCK_HZ; 
  const uint64_t ticks_per_minute = (tick_Hz * 60) ; 
  uint64_t tick_diff_sum;
  uint64_t tick_diff_average;
  uint32_t rpsx100;
  uint32_t rpm;
  
  BaseType_t retval;

  /* Filling up history */
  if (1 < missing)
	{
	  prvHistory[start + prvHMAX - missing] = tse_buf;
	  missing -= 1;
	  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Missing values  ");
	  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%d               ", missing);
	  return;
	}
  /* Got enough history  */
  if (1 == missing)
	{
	  prvHistory[start + prvHMAX - missing] = tse_buf;
	  missing -= 1;
	}
  else
	{
	  prvHistory[start & (prvHMAX - 1)] = tse_buf;
      start++;
	}
  /* 
   * Averaging longer diffs better than "(last - first) / N" 
   * Easy way out - require prvHmax even
   */
  tick_diff_sum = 0;
  for(j = 0; j < prvHMAX/2 ; j += 1)
	{
	  h0 = (start + j ) & (prvHMAX - 1);
	  h1 = (start + prvHMAX/2 + j ) & (prvHMAX - 1);

	  /* 
	   * Accepting minutCount diff of 2 is not really reasonable.
	   * It does allow drop-through though :-)
	   */
	  switch (prvHistory[h1].minuteCount - prvHistory[h0].minuteCount)
		{
		case 2:
		  tick_diff_sum += timer1_TICKS_PER_MINUTE;
		case 1:
		  tick_diff_sum += timer1_TICKS_PER_MINUTE;
		case 0:
		  tick_diff_sum += (prvHistory[h1].captureCount - prvHistory[h0].captureCount);
		  break;
		default:
		  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Invalid mC diff.");
		  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%08X            ",
				   prvHistory[h1].minuteCount - prvHistory[h0].minuteCount);            
		  return;
		}
	}
  /* number of intervals in each diff = (prvHMAX/2) */
  /* number of such diffs summed = (prvHMAX/2)*/
  tick_diff_average = tick_diff_sum / (prvHMAX/2) / (prvHMAX/2);
  rpsx100 = (uint32_t)  ( (100 * tick_Hz)  / tick_diff_average );
  
  rpm = (uint32_t) (ticks_per_minute  / tick_diff_average);
  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "" xstr(CGRAM_r) xstr(CGRAM_p) xstr(CGRAM_m) "    " xstr(CGRAM_r) xstr(CGRAM_p) xstr(CGRAM_s) "     ");
  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%3d   %02d.%02d       ", rpm, rpsx100/100, (rpsx100- 100 * (rpsx100/100)));
  return;
}
#undef prvHMAX

static void prvShowSpeed(void)
{
  xQueueSend(LCDQ, &prvSpeedToShowLine1, portMAX_DELAY);
  xQueueSend(LCDQ, &prvSpeedToShowLine2, portMAX_DELAY);
}

static void prvSpeedUpdate(timeStoreElement_t tse_buf)
{
  prvSpeedLog(tse_buf);
  prvSpeedCalc(tse_buf);
  prvShowSpeed();
}
