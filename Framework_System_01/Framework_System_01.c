#include "Framework_System_01.h"

UINT16 MilliSeconds_Timer = 0;

void RTC_Isr(void)   
{
  RTC_CLEAR_FLAG();
  Midware_Led_1ms_Int();
//  Midware_Incline_1ms_Int();    
//  Midware_Digital_1ms_Int();
//  Midware_Induction_1ms_Int(); 
//  Midware_ECB_1ms_Int();
  Midware_RPM_1ms_Int();
  Midware_Timer_1ms_Int();
//  Midware_Update_1ms_Int();
  MilliSeconds_Timer++;
}

UINT16 MilliSeconds_Timer_Get(void)
{
  return MilliSeconds_Timer;
}

void UART1_Isr(void)
{ 
  Hal_Digital_Int();
}

void KBI0_Isr(void)	
{
  Hal_ECB_INT();
}

void KBI1_Isr(void)	
{
  Hal_RPM_INT();
}

void NMI_Isr(void)
{
  NMI_DISABLE();
}              

void ACMP1_Isr(void)
{
  Hal_Incline_Int();
}

void ADC_Isr(void)
{
}

void FTM0_Isr(void)
{
}

void FTM2_Isr(void)
{
}

void UART0_Isr(void)
{
}

void UART2_Isr(void)
{
}

void FTM1_Isr(void)
{  
  Hal_RPM_INT0();
}

void ACMP0_Isr(void)
{
    if(ACMP_GetFlag(ACMP0))
    { 
      ACMP_ClrFlag(ACMP0);  
    }
}

void Midware_System_Initial_HW(void)
{
  Hal_System_Initial();
}

void Midware_System_EnableRTC(void)
{
  Hal_System_EnableRTC();
}

void Midware_System_ERP(void)
{   
  Hal_System_ERP(); 
}

void Midware_System_WDOGFeed(void)
{   
  Hal_System_WDOGFeed(); 
}

