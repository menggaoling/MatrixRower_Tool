#ifndef __SYSTEM_H__
#define __SYSTEM_H__


void Midware_System_Initial_HW(void);
void Midware_System_WDOG_Feed(void);

void Midware_System_ERP(void);
void Midware_System_EnableRTC(void);
void RTC_Isr(void);
void FTM0_Isr(void);
void FTM1_Isr(void);
void FTM2_Isr(void);
void UART0_Isr(void);
void UART1_Isr(void);
void UART2_Isr(void);


#endif

