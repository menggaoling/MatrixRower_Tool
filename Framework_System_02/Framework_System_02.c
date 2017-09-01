#include "Framework_System_02.h"
#include "common.h"
#include "rtc.h"
#include "Midware_Timer_01.h"
#include "Midware_Motor_01.h"
#include "Midware_RPM_MCB_01.h"
#include "Midware_Digital_01.h"
#include "Midware_Led_01.h"
#include "Hal_Digital_01.h"
#include "Hal_Trace_01.h"
#include "Midware_Trace_01.h"
#include "Midware_Temp_01.h"
#include "Midware_Incline_01.h"
#include "Midware_Update_01.h"
#include "Midware_Safekey_01.h"
#include "Midware_BCS.h"



void Midware_System_Initial_HW(void)
{
    Hal_System_Initial();
}
void Midware_System_WDOG_Feed(void)
{
    Hal_System_WDOG_Feed();
}
void Midware_System_ERP(void)
{
    Hal_System_ERP();
}

void Midware_System_EnableRTC(void)
{
    Hal_System_EnableRTC();
}

void RTC_Isr(void)
{
    RTC_ClrFlags();
    
    Midware_Safekey_1ms_Int();
    Midware_Motor_1ms_Int();
    Midware_Digital_1ms_Int();
    Midware_Incline_1ms_Int();
    Midware_BCS_1ms_Int();
    Midware_Update_1ms_Int();
    Midware_Led_1ms_Int();
    Midware_Timer_1ms_Int();
    Midware_Trace_1ms_Int();
    Midware_Temp_1ms_Int();
}

void UART0_Isr(void)
{
    
}

void UART1_Isr(void)
{
    Hal_Digital_Int();
}

void UART2_Isr(void)
{
  
}

void FTM0_Isr(void)
{

}

void FTM1_Isr(void)
{

}

void FTM2_Isr(void)
{
    Hal_RPM_INT();
}

void ADC_Isr(void)
{
}

void ACMP1_Isr(void)
{
    Hal_Incline_Int();
}


