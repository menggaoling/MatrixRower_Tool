#ifndef __FRAMEWORK_SYSTEM_01_H__
#define __FRAMEWORK_SYSTEM_01_H__

#include "Hal_System_01.h"

void RTC_Isr(void);
void FTM0_Isr(void);
void FTM1_Isr(void);
void FTM2_Isr(void);
void UART0_Isr(void);
void UART1_Isr(void);
void UART2_Isr(void);
void KBI0_Isr(void);
void ACMP0_Isr(void);
void ACMP1_Isr(void);
void Midware_System_ERP(void);
void Midware_System_Initial_HW(void);
void Midware_System_EnableRTC(void);
void Midware_System_WDOGFeed(void);
UINT16 MilliSeconds_Timer_Get(void);

#endif 