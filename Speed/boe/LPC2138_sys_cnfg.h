/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2005
 *
 *    File name   : LPC2138_sys_cnfg.h
 *    Description : System definitions
 *
 *    History :
 *    1. Date        : Feb 10, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28 $
 **************************************************************************/

#ifndef __LPC2138_SYS_CNFG_H
#define __LPC2138_SYS_CNFG_H

/* OSC [Hz] */
#define FOSC 	            14745600
/* Core clk [Hz] */
#define FCCLK	            14745600
//#define FCCLK	            14745600 * 5
/* Time Precision time [us] */
#define TIMER_PRECISION   1
/* Sys timer tick per seconds */
#define TICK_PER_SECOND   100
/* MAM Initialization State */
#define MAM_CTRL          MAM_FULL
/* MAM Initialization Timing*/
//BoE
//#define MAM_DIV           MAMCycle3
#define MAM_DIV           MAMCycle1
#endif /* __LPC2138_SYS_CNFG_H */


