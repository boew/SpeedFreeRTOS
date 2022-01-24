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

/* Standard includes. */
#include <stdlib.h>

#include "FreeRTOS.h"
#include "boe/timer1.h"

/* Constants for the ComTest tasks. */
#define mainTX_ENABLE				( ( unsigned long ) 0x0001 )
#define mainRX_ENABLE				( ( unsigned long ) 0x0004 )

/* Constants to setup the PLL. */
#define mainPLL_MUL_5				( ( unsigned char ) 0x0004 )
#define mainPLL_MUL_4				( ( unsigned char ) 0x0003 )
#define mainPLL_DIV_1				( ( unsigned char ) 0x0000 )
#define mainPLL_ENABLE				( ( unsigned char ) 0x0001 )
#define mainPLL_CONNECT				( ( unsigned char ) 0x0003 )
#define mainPLL_FEED_BYTE1			( ( unsigned char ) 0xaa )
#define mainPLL_FEED_BYTE2			( ( unsigned char ) 0x55 )
#define mainPLL_LOCK				( ( unsigned long ) 0x0400 )

/* Constants to setup the MAM. */
#define mainMAM_TIM_3				( ( unsigned char ) 0x03 )
#define mainMAM_MODE_FULL			( ( unsigned char ) 0x02 )

/* Constants to setup the peripheral bus. */
#define mainBUS_CLK_FULL			( ( unsigned char ) 0x01 )

/* And finally, constant to setup the port for the LED's. */
#define mainLED_TO_OUTPUT			( ( unsigned long ) 0x00ff0000 )

/*
 * Configures the processor for use with this demo.
 */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler.
 */
void main( void )
{
	/* Setup the processor. */
	prvSetupHardware();

	/* Start all the standard demo application tasks. */
#define  SPEED138  1
#if SPEED138 
        setupTimer1();
        vAltStartQTestTask( ( UBaseType_t ) 3, (uint32_t) 115200 ) ; // 115200 OK x-lu, 230400 460800 OK new win10 machine too );
#else        
	vStartIntegerMathTasks( tskIDLE_PRIORITY );
	vStartLEDFlashTasks( mainLED_TASK_PRIORITY );
	vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
	vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
	vStartDynamicPriorityTasks();
	vAltStartComTestTasks( mainCOM_TEST_PRIORITY, mainCOM_TEST_BAUD_RATE, mainCOM_TEST_LED );
		
	/* Start the check task - which is defined in this file. */
	xTaskCreate( vErrorChecks, "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );
#endif
	/* Start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here.
	*/

	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler. */
  	return;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
 
	/* Setup the PLL to multiply the XTAL input by 4. */
    PLLCFG = ( mainPLL_MUL_4 | mainPLL_DIV_1 );
	/* Activate the PLL by turning it on then feeding the correct sequence of
	bytes. */
	PLLCON = mainPLL_ENABLE;
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;
	/* Wait for the PLL to lock... */
	while( !( PLLSTAT & mainPLL_LOCK ) );

	/* ...before connecting it using the feed sequence again. */
	PLLCON = mainPLL_CONNECT;
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;

	/* Setup and turn on the MAM.  Three cycle access is used due to the fast
	PLL used.  It is possible faster overall performance could be obtained by
	tuning the MAM and PLL settings. */
	MAMTIM = mainMAM_TIM_3;
	MAMCR = mainMAM_MODE_FULL;

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
	
	/* Configure the RS2332 pins.  All other pins remain at their default of 0. */
	PINSEL0 |= mainTX_ENABLE;
	PINSEL0 |= mainRX_ENABLE;

	/* LED pins need to be output. */
	IO1DIR = mainLED_TO_OUTPUT;

}
