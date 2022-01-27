/***************************************************************************
 **
 **    Master include file
 **
 **    Used with ARM IAR C/C++ Compiler and Assembler.
 **
 **    (c) Copyright IAR Systems 2005
 **
 **    $Revision: 28 $
 **
 ***************************************************************************/
#ifndef  __INCLUDES_H
#define  __INCLUDES_H

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <stdbool.h>

#include    <intrinsics.h>
#include    <NXP/iolpc2148.h>

#include    "LPC_data_type.h"

#if 0
#include    "arm_comm.h"
#include    "LPC2138_sys_cnfg.h"

#include    "menu_cnfg.h"
#include    "menu.h"
#endif 
#include    "drv_hd44780.h"
#if 0
#include    "LPC_SysControl.h"
#include    "LPC_Vic.h"
#include    "LPC_Uart.h"
#include    "LPC_Timer.h"
#include    "LPC_Rtc.h"
#endif
#endif // __INCLUDES_H
