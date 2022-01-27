/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2005
 *
 *    File name   : drv_hd44780_l.c
 *    Description : I/O function
 *
 *    History :
 *    1. Date        : June 13, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28 $
**************************************************************************/
#include "drv_hd44780_cnfg.h"
#include "drv_hd44780_l.h"

#define HD44780_INPUT_SET_MASK        ~0x00003C00
#define HD44780_OUTPUT_SET_MASK        0x84803C00

#define HD44780_DATA_BITS             10
#define HD44780_RS_BIT                23
#define HD44780_E_BIT                 26
#define HD44780_RW_BIT                31

/*************************************************************************
 * Function Name: HD44780_IO_Init
 * Parameters: none
 * Return: none
 * Description: Init IO ports directions and level
 *
 *************************************************************************/
void HD44780_IO_Init (void)
{
  PINSEL0 &= ~0x0FF00000;
  PINSEL1 &= ~0xC030C000;
  IO0DIR  |= HD44780_OUTPUT_SET_MASK;
  // RS Output - Low
  HD44780SetRS(0);
  // E  Output - Low
  HD44780SetE(0);
#if HD4780_WR > 0
  // WR Output - Low
  HD44780SetRW(0);
#endif
}

/*************************************************************************
 * Function Name: HD44780SetRS
 * Parameters: Boolean Data
 * Return: none
 * Description: Set RS signal
 *
 *************************************************************************/
void HD44780SetRS (Boolean Data)
{
  if(Data)
  {
    IO0SET = 1UL<<HD44780_RS_BIT;
  }
  else
  {
    IO0CLR = 1UL<<HD44780_RS_BIT;
  }
}

/*************************************************************************
 * Function Name: HD44780SetE
 * Parameters: Boolean Data
 * Return: none
 * Description: Set E signal
 *
 *************************************************************************/
void HD44780SetE (Boolean Data)
{
  if(Data)
  {
    IO0SET = 1UL<<HD44780_E_BIT;
  }
  else
  {
    IO0CLR = 1UL<<HD44780_E_BIT;
  }
}

/*************************************************************************
 * Function Name: HD44780WrIO
 * Parameters: uint8_t Data
 * Return: none
 * Description: Write to HD44780 I/O
 *
 *************************************************************************/
void HD44780WrIO (uint8_t Data)
{
uint32_t Tmp = (uint32_t)Data << 10;
  // Set Direction
  IO0DIR  |= HD44780_OUTPUT_SET_MASK;
#if HD4780_WR > 0
  HD44780SetRW(0);
#endif
  // Write Data
  // Clock E
  HD44780SetE(1);

  IO0SET |=  Tmp & 0x3C00;
  IO0CLR |= ~Tmp & 0x3C00 ;
  HD44780_BUS_DLY();
  HD44780SetE(0);
}

#if HD4780_WR > 0
/*************************************************************************
 * Function Name: HD44780SetRW
 * Parameters: Boolean Data
 * Return: none
 * Description: Set R/W signal
 *
 *************************************************************************/
void HD44780SetRW (Boolean Data)
{
  if(Data)
  {
    IO0SET = 1UL<<HD44780_RW_BIT;
  }
  else
  {
    IO0CLR = 1UL<<HD44780_RW_BIT;
  }
}

/*************************************************************************
 * Function Name: HD44780RdIO
 * Parameters: none
 * Return: uint8_t
 * Description: Read from HD44780 I/O
 *
 *************************************************************************/
uint8_t HD44780RdIO (void)
{
uint8_t Data;
  // Set Direction
  IO0DIR  &= HD44780_INPUT_SET_MASK;
  HD44780SetRW(1);
  // Read Data
  HD44780SetE(1);
  HD44780_BUS_DLY();
  Data = (IO0PIN >> 10) & 0xF;
  HD44780SetE(0);
  // Clock E
  return Data;
}
#endif
