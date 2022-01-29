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
#define timestoretestBUFFERSIZE 800
#define prvPRINTBUFFERSIZE 80
#define prvSIGNON "qC\tmC\tcC\t\tREFE\r\n"
#define prvREV_TIMEOUT_MS 1000

/*-----------------------------------------------------------*/

static portTASK_FUNCTION_PROTO( vSpeedTask, pvParameters );
static lcdToShow_t prvSpeedToShow = {1,1, " --- "};
static void prvShowSpeed(void);
static void prvSpeedUpdate(timeStoreElement_t tse_buf);
static void prvSpeedLog(timeStoreElement_t tse_buf);

void vStartSpeedTask( UBaseType_t uxPriority, uint32_t ulBaudRate)
{
  xPort = xSerialPortInitMinimal( ulBaudRate, timestoretestBUFFERSIZE );  
  xTaskCreate( vSpeedTask, "Speed", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL ); 
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
  retval = sprintf(&prvPrintBuffer, "mC=%03d\tcC=0x%08X\tREFE:0x%08X\r\n",
                     tse_buf.minuteCount, tse_buf.captureCount, tse_buf.REFE & (1 << 17));
  vSerialPutString(xPort, prvPrintBuffer, prvPRINTBUFFERSIZE);
}
static void prvShowSpeed(void)
{
  xQueueSend(LCDQ, &prvSpeedToShow, portMAX_DELAY);
}

static void prvSpeedUpdate(timeStoreElement_t tse_buf)
{
  prvSpeedLog(tse_buf);
  prvShowSpeed();
}