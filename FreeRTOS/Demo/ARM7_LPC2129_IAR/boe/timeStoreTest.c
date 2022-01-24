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

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME			( ( TickType_t ) 0xffff )

/* Handle to the com port used by both tasks. */
static xComPortHandle xPort = NULL;
#define  timestoretestBUFFERSIZE 800
#define  prvPRINTBUFFERSIZE 80


/*-----------------------------------------------------------*/

static portTASK_FUNCTION_PROTO( vQTestTask, pvParameters );

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate)
{
  	xPort = xSerialPortInitMinimal( ulBaudRate, timestoretestBUFFERSIZE );
	xTaskCreate( vQTestTask, "QTT", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
}

static timeStoreElement_t tse_buf;
static signed char prvPrintBuffer[prvPRINTBUFFERSIZE];
volatile uint32_t qcount=0;
static portTASK_FUNCTION( vQTestTask, pvParameters )
{
  TickType_t xTimeToBlock;
  TickType_t xTimeToWait;
  BaseType_t retval;
  xTimeToBlock = portMAX_DELAY; // ( TickType_t ) 0x32 ; 
  xTimeToWait =  ( TickType_t ) 0x32 ; 
	/* Just to stop compiler warnings. */
	( void ) pvParameters;
	for( ;; )
	{
	  retval = xQueueReceive(timeStore, (void*) &tse_buf, xTimeToWait); // xTimeToWait xTimeToBlock);
	  retval = sprintf(&prvPrintBuffer, "qC=%03d\tmC=%03d\tcC=0x%08X\r\n",
					   qcount, tse_buf.minuteCount, tse_buf.captureCount);
	  vSerialPutString(xPort, prvPrintBuffer, prvPRINTBUFFERSIZE);
	  qcount++;
	}
} /*lint !e715 !e818 pvParameters is required for a task function even if it is not referenced. */

/*-----------------------------------------------------------*/

