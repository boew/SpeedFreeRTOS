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


/*
 * This version of comtest. c is for use on systems that have limited stack
 * space and no display facilities.  The complete version can be found in
 * the Demo/Common/Full directory.
 *
 * Creates two tasks that operate on an interrupt driven serial port.  A
 * loopback connector should be used so that everything that is transmitted is
 * also received.  The serial port does not use any flow control.  On a
 * standard 9way 'D' connector pins two and three should be connected together.
 *
 * The first task posts a sequence of characters to the Tx queue, toggling an
 * LED on each successful post.  At the end of the sequence it sleeps for a
 * pseudo-random period before resending the same sequence.
 *
 * The UART Tx end interrupt is enabled whenever data is available in the Tx
 * queue.  The Tx end ISR removes a single character from the Tx queue and
 * passes it to the UART for transmission.
 *
 * The second task blocks on the Rx queue waiting for a character to become
 * available.  When the UART Rx end interrupt receives a character it places
 * it in the Rx queue, waking the second task.  The second task checks that the
 * characters removed from the Rx queue form the same sequence as those posted
 * to the Tx queue, and toggles an LED for each correct character.
 *
 * The receiving task is spawned with a higher priority than the transmitting
 * task.  The receiver will therefore wake every time a character is
 * transmitted so neither the Tx or Rx queue should ever hold more than a few
 * characters.
 *
 */

/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/* Demo program include files. */
#include "serial.h"
#include "comtest.h"
#include "partest.h"
#include "queue.h"
#include "FreeRTOS.h"
#include "timer1.h"

#define comSTACK_SIZE				configMINIMAL_STACK_SIZE
#define comTX_LED_OFFSET			( 0 )
#define comRX_LED_OFFSET			( 1 )
#define comTOTAL_PERMISSIBLE_ERRORS ( 2 )

/* The Tx task will transmit the sequence of characters at a pseudo random
interval.  This is the maximum and minimum block time between sends. */
#define comTX_MAX_BLOCK_TIME		( ( TickType_t ) 0x96 )
#define comTX_MIN_BLOCK_TIME		( ( TickType_t ) 0x32 )
#define comOFFSET_TIME				( ( TickType_t ) 3 )

/* We should find that each character can be queued for Tx immediately and we
don't have to block to send. */
#define comNO_BLOCK					( ( TickType_t ) 0 )

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME			( ( TickType_t ) 0xffff )

/* Handle to the com port used by both tasks. */
static xComPortHandle xPort = NULL;
#define  TIMEPRINTBUFFER_MAX 800


/*-----------------------------------------------------------*/

static portTASK_FUNCTION_PROTO( vQTestTask, pvParameters );

void vAltStartQTestTask( UBaseType_t uxPriority, uint32_t ulBaudRate)
{
  	xPort = xSerialPortInitMinimal( ulBaudRate, TIMEPRINTBUFFER_MAX );
	xTaskCreate( vQTestTask, "QTT", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
}

static timeStoreElement_t tse_buf;
static signed char timePrintBuffer[TIMEPRINTBUFFER_MAX]="PQRSTUVWXYZ[\\]";
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
	  //printf("received retval: %d minutes: %d captured: %d\n", retval, tse_buf.minuteCount, tse_buf.captureCount);
	  retval = sprintf(&timePrintBuffer, "qcount %03d\t", qcount);
	  vSerialPutString(xPort, timePrintBuffer, TIMEPRINTBUFFER_MAX);
	  //vTaskDelay( xTimeToWait );
	  retval = sprintf(&timePrintBuffer, "GPIO Edge %01d\t", (tse_buf.minuteCount & 0x02U << 30) >> 31);
	  vSerialPutString(xPort, timePrintBuffer, TIMEPRINTBUFFER_MAX);
	  retval = sprintf(&timePrintBuffer, "MC %03d\t", (tse_buf.minuteCount & ((0x02U << 30) - 1)));
	  vSerialPutString(xPort, timePrintBuffer, TIMEPRINTBUFFER_MAX);          
	  retval = sprintf(&timePrintBuffer, "CC 0x%08X\r\n", tse_buf.captureCount);
	  vSerialPutString(xPort, timePrintBuffer, TIMEPRINTBUFFER_MAX);
	  qcount++;
	}
} /*lint !e715 !e818 pvParameters is required for a task function even if it is not referenced. */

/*-----------------------------------------------------------*/

