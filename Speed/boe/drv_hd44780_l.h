/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : drv_hd44780_l.h
 *    Description : LCD low level I/O function inlcude file
 *
 *    History :
 *    1. Date        : June 13, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28 $
 **************************************************************************/
#include "includes.h"

#ifndef  __DRV_HD44780_L_H
#define  __DRV_HD44780_L_H

/* Delay macro */
#define HD44780_BUS_DLY()      for(volatile int dly = 40;dly;--dly) //Was 10, then clock x4

void HD44780_IO_Init (void);
void HD44780SetRS (Boolean Data);
void HD44780SetE (Boolean Data);
void HD44780WrIO (uint8_t);
void HD44780SetRW (Boolean Data);
uint8_t HD44780RdIO (void);

#endif // __DRV_HD44780_L_H
