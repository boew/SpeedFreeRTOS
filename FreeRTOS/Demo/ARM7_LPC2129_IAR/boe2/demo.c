/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2003
 *
 *    File name   : main.c
 *    Description : Main module
 *
 *    History :
 *    1. Date        : Feb 10, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 * Buttons function
 * Butt1 - Next
 * Butt2 - Select
 *
 * Terminal settings:
 * UART1
 * ------------------
 * Communication speed: 9600 bps
 * Number of data bits:    8
 * Number of stop bits:    1
 * Parity:              none
 *
 * UART2
 * ------------------
 * Communication speed: 9600 bps
 * Number of data bits:    8
 * Number of stop bits:    1
 * Parity:              none
 *
 * Jumper settings:
 * ----------------
 *
 *
 * Set PWM/DAC to DAC.
 * Set Ext/J-Link to Ext.
 *
 *
 *
 *    $Revision: 28 $
**************************************************************************/
#include "includes.h"
#include <user_func.h>

#ifdef FLASH
#pragma segment=".crp"
#pragma location=".crp"
__root const unsigned int crp = 0xFFFFFFFF;
#endif

#define LIGHT_ALWAYS_OFF  0
#define LIGHT_AUTO        1
#define LIGHT_ALWAYS_ON   2
#define LIGHT_AUTO_OFF_TO TICK_PER_SECOND*10 /* 10sec */

#define GET_TIME          '0'
#define GET_DATE          '1'
#define GET_LEN_PULSE     '2'
#define GET_OFFSET_T      '3'
#define SET_TIME          '4'
#define SET_DATE          '5'
#define SET_LEN_R         '6'
#define SET_OFFSET_T      '7'



const char UART_Menu[] = "\r\nUART commands:\r\n" 
                         "'0' Get Time\r\n"
                         "'1' Get Date\r\n"
                         "'2' Get Length per pulse\r\n"
                         "'3' Get Offset for Trip\r\n"
                         "'4' Set Time\r\n"
                         "'5' Set Date\r\n"
                         "'6' Set Length per pulse\r\n"
                         "'7' Set Offset for Trip\r\n"
                         "'?' Help\n\r";

volatile int TickSysFlag = 0;
int SysTimeUpdateFlag = 0;
int SysAlarmFlag = 0;

int B1_Short = 0;
int B2_Short = 0;

int TimeFormat = 1;
int DataFormat = 1;

int LightMode = LIGHT_ALWAYS_ON;

/*************************************************************************
 * Function Name: IRQSub
 * Parameters: void
 * Return: void
 *
 * Description: IRQ subroutine
 *		
 *************************************************************************/
__irq __arm void IRQ_Handler (void)
{
void (*interrupt_function)();
unsigned int vector;

  vector = VICVectAddr;     // Get interrupt vector.
  interrupt_function = (void(*)())vector;
  (*interrupt_function)();  // Call vectored interrupt function.
}

/*************************************************************************
 * Function Name: IRQSub
 * Parameters: void
 * Return: void
 *
 * Description: FIQ subroutine
 *		
 *************************************************************************/
__fiq __arm void FIQ_Handler (void)
{
  while(1);
}

/*************************************************************************
 * Function Name: NonVectISR
 * Parameters: void
 * Return: void
 *
 * Description: non vectored callback subroutine
 *		
 *************************************************************************/
void NonVectISR(void)
{
}

/*************************************************************************
 * Function Name: ClearFlag
 * Parameters: void
 * Return: void
 *
 * Description: clear arg
 *		
 *************************************************************************/
void ClearFlag (void* arg)
{
int* pFlag = arg;
  *pFlag = 0;
}

/*************************************************************************
 * Function Name: DelayResolution100us
 * Parameters: Int32U Delay
 * Return: void
 *
 * Description: Timer1 CH0 subroutine - delay [100us]
 *		
 *************************************************************************/

void DelayResolution100us(Int32U Delay)
{
  volatile int Flag = 1;
  // current Timer Count as starting point
  unsigned long tcNow = TIMER_GetREGValue_TC(TIMER1);
  // add delay; wrapping should be OK (same type!)
  unsigned long matchCount = tcNow + Delay usec_T1*100;
  // Set action of match module CH0
  TIMER_SetMatchAction(TIMER1, CH0, TimerAction_Interrupt,
		       matchCount, ClearFlag, (void *)&Flag, DONOTHING);
  // Wait expire of delay
  while(Flag);
}
#define BoE 1
#if BoE
void BoE_UART_PutCharByPolling (char ch)
{
  while(!U0LSR_bit.THRE);
  U0THR = ch;
}

int BoE_UART_PutStringByPolling(char *Buf)
{
  char *pBuf = Buf ;
  int SendCount = 0;
  while (*pBuf)
    {
      BoE_UART_PutCharByPolling(*pBuf++);
      ++SendCount;
    }
  return (SendCount);
}
#endif 
/*************************************************************************
 * Function Name: SysInit
 * Parameters: void
 * Return: int
 *
 * Description: Hardware initialize
 *		
 *************************************************************************/
int SysInit(void)
{
  LPC_SysControl_MAMConfig_t LPC_SysControl_MAMConfig = {MAM_CTRL,MAM_DIV};
  // Initialize the system
#ifdef FLASH
  if (SYS_Init(FOSC, FCCLK, VPBDIV1, USER_FLASH,MAM_CTRL,&LPC_SysControl_MAMConfig,
               0x0001FF0F,0x87FE01F1,0,0xFFFFFFFF))
    return 1;
#else
  if (SYS_Init(FOSC, FCCLK, VPBDIV1, USER_RAM,  MAM_CTRL,&LPC_SysControl_MAMConfig,
               0x0001FF0F,0x87FE01F1,0,0xFFFFFFFF))
    return 1;
#endif
  // Initialize Serial Interface
  
#if BoE
  UART0_Init();
  BoE_UART_PutStringByPolling("\n\rLPC2138 Hardware Init Finish!\n\r");
  BoE_UART_PutStringByPolling("======Start User Program======\n\r");
  while(1) { ; }
#else  
  if (UART_Init(UART0))
    return 1;
  if (UART_Init(UART1))
    return 1;
#endif 
  
  // Initialize Timers
  if (TIMER_Init(TIMER0, TIMER_PRECISION))
    return 1;
  if (TIMER_Init(TIMER1, TIMER_PRECISION))
    return 1;

  // Stop Timer 1
  TIMER_Stop(TIMER1);
  // Stop Reset Timer 1 counter
  TIMER_Reset(TIMER1);
  // Set action of match module CH0
  // Start Timer 1
  TIMER_Start(TIMER1);
  
  // Initialize RTC
  if (RTC_Init(0))	
    return 1;

  // initialize VIC
  VIC_Init();
  VIC_SetProtectionMode(UserandPrivilegedMode);
  // Enable interrupts non vectored interrupts
  VIC_EnableNonVectoredIRQ(NonVectISR);

  // UART0 interrupt
  VIC_SetVectoredIRQ(UART0_ISR,VIC_Slot0,VIC_UART0);
  VIC_EnableInt(1<<VIC_UART0);

  // UART1 interrupt
  VIC_SetVectoredIRQ(UART1_ISR,VIC_Slot1,VIC_UART1);
  VIC_EnableInt(1<<VIC_UART1);

  // Timer0 interrupt
  VIC_SetVectoredIRQ(TIMER0_ISR,VIC_Slot2,VIC_TIMER0);
  VIC_EnableInt(1<<VIC_TIMER0);

  // Timer1 interrupt
  VIC_SetVectoredIRQ(TIMER1_ISR,VIC_Slot3,VIC_TIMER1);
  VIC_EnableInt(1<<VIC_TIMER1);

  // UART1 interrupt
  VIC_SetVectoredIRQ(UART1_ISR,VIC_Slot4,VIC_UART1);
  VIC_EnableInt(1<<VIC_UART1);

  // RTC interrupt
  VIC_SetVectoredIRQ(RTC_ISR,VIC_Slot5,VIC_RTC);
  VIC_EnableInt(1<<VIC_RTC);

  UART_PutStringByPolling(UART0, "\n\r YOW LPC2138 Hardware Init Finish!\n\r");
  
  return 0;
}

/*************************************************************************
 * Function Name: UserStart
 * Parameters: void
 * Return: void
 *
 * Description: User demo subroutine
 *		
 *************************************************************************/
void UserStart(void)
{
  /* System time init */
  TIMER_SetMatchAction(TIMER0, CH0, TimerAction_Interrupt | TimerAction_ResetTimer,
  1sec_T0/TICK_PER_SECOND, SetSysTickFlag, (void *)&TickSysFlag, DONOTHING);
  TIMER_Start(TIMER0);
  /* HD44780 LCD driver init */
  HD44780_PowerUpInit();
  /* Buttons Init */
  ButtonsInit();
  /* Light Init */
  LightInit();
  /* Menu Init */
  MenuInit(MENU_ENGLISH_LANG,MENU_WELCOME_SHOW,NULL,MENU_WELCOM_DLY);
  /* RTC_Enable */
  RTC_Enable();
}

/*************************************************************************
 * Function Name: main
 * Parameters: void
 * Return: void
 *
 * Description: Main subroutine
 *		
 *************************************************************************/
int main (void)
{
LPC_INT8U Temp;
LPC_INT16U ScreenSaver = MENU_LOW_POWER_DLY;
LPC_INT16U LightAutoOffCount = 0;
char RecData, TranBuff[30];
LPC_Rtc_Time_t CurrTime;
LPC_Rtc_Date_t CurrData;

  if(SysInit() == 0)
  {
    UART_PutStringByPolling(UART1,"\n\rWas 0\n\r");
    // Start user program
    __enable_interrupt();
    UART_PutStringByPolling(UART1,"\n\rEnabled 1\n\r");
    UserStart();
    UART_PutStringByPolling(UART1,"\n\rStarted 2\n\r");
  }
  UART_PutString(UART0,(char*)UART_Menu);
  UART_PutString(UART1,(char*)UART_Menu);
  MenuSetEvent(MENU_LAST_IND);
  /* CPCH2 = CAP1.2 = pin 0.17 */
  TIMER_SetCaptureAction (TIMER1, CPCH2, 
//                          TimerCPTrigger_Rising+TimerCPTrigger_Falling,
                          TimerCPTrigger_Rising,
			  1, DoTimes, &CaptureCount);
  while(1)
  {
    if(TickSysFlag)
    {
      TickSysFlag = 0;
      /* Sys tick event */
      MenuSetEvent(MENU_TICK_EVENT);
      /* Screensaver Time out count */
      if (MenuGetInd() == MENU_SCREENSAVER_SHOW)
      {
        if(ScreenSaver-- == 0)
        {
//          MenuSetEvent(MENU_EVENT_LOW_POWER);
        }
      }
      /* ADC measure event */
      if (MenuGetInd() == MENU_ADC_MEASURE)
      {
        MenuSetEvent(MENU_EVENT_ADC_CONV);
      }
      else
      {
        ScreenSaver = MENU_LOW_POWER_DLY;
      }
      /* Light control */
      if (LightMode == LIGHT_ALWAYS_OFF)
      {
        Temp = LIGHT_OFF;
      }
      else if (LightMode == LIGHT_ALWAYS_ON)
      {
        Temp = LIGHT_ON;
      }
      else
      {
        if(LightAutoOffCount)
        {
          --LightAutoOffCount;
          Temp = LIGHT_ON;
        }
        else
        {
          Temp = LIGHT_OFF;
        }
      }
      LightCntr(LIGHT_SLOW,(LPC_BOOL)Temp);
    }
    /* Time and Date update event */
    if(SysTimeUpdateFlag)
    {
      SysTimeUpdateFlag = 0;
      MenuSetEvent(MENU_EVENT_TIME_UPDATE);
    }
    /* Buttons event */
    Temp = GetButtonsEvent();
    if (Temp != 0)
    {
      LightAutoOffCount = LIGHT_AUTO_OFF_TO;
    }
    /* Button1 event "Next" */
    if (Temp&BUTT_EVENT_1)
    {
      MenuSetEvent(MENU_EVENT_B1_SHORT_PRESS);
    }
    /* Button2 event "Select" */
    if (Temp&BUTT_EVENT_2)
    {
      MenuSetEvent(MENU_EVENT_B2_SHORT_PRESS);
    }
    /* UART0 */
    if(UART_GetCharByInterrupt(UART0,&RecData) != 0)
    {
      switch (RecData)
      {
      case GET_TIME:
        RTC_GetTime(&CurrTime);
        FormatTime(TimeFormat,&CurrTime,TranBuff);
        UART_PutString(UART0,TranBuff);
        UART_PutString(UART0,(char*)UART_Menu);
        break;
      case GET_DATE:
        RTC_GetDate(&CurrData);
        FormatDate(DataFormat,&CurrData,TranBuff);
        UART_PutString(UART0,TranBuff);
        UART_PutString(UART0,(char*)UART_Menu);
        break;
      default:
        UART_PutString(UART0,(char*)UART_Menu);
        break;
      }
    }
    /* UART1 */
    if(UART_GetCharByInterrupt(UART1,&RecData) != 0)
    {
      switch (RecData)
      {
      case GET_TIME:
        RTC_GetTime(&CurrTime);
        FormatTime(TimeFormat,&CurrTime,TranBuff);
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case GET_DATE:
        RTC_GetDate(&CurrData);
        FormatDate(DataFormat,&CurrData,TranBuff);
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case GET_LEN_PULSE:
        //sprintf(TranBuff, "GET_LEN_PULSE");
        UART_PutString(UART1,"GET_LEN_PULSE");
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case GET_OFFSET_T:
        sprintf(TranBuff, "Trip Offset %lld", TripOffset);
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case SET_TIME:
        RTC_GetTime(&CurrTime);
        FormatTime(TimeFormat,&CurrTime,TranBuff);
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case SET_DATE:
        RTC_GetDate(&CurrData);
        FormatDate(DataFormat,&CurrData,TranBuff);
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case 99:
        //sprintf(TranBuff, "GET_LEN_PULSE");
        UART_PutString(UART1,"SET_LEN_PULSE");
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      case 100:
        sprintf(TranBuff, "Set Trip Offset");
        UART_PutString(UART1,TranBuff);
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      default:
        UART_PutString(UART1,(char*)UART_Menu);
        break;
      }
    }

    /* Menu Implement */
    Menu();
  }
}
