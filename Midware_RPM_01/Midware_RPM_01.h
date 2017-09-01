#ifndef __MIDWARE_RPM_01_H__
#define __MIDWARE_RPM_01_H__

#include "Hal_RPM_01.h"

void Midware_RPM_Initial_HW(void);
void Midware_RPM_Initial_Data(void);
void Midware_RPM_1ms_Int(void);   
void Midware_RPM_Process(void);
UINT16 Midware_RPM_Get_RPM(void);
void Midware_RPM_Erp(void);
void Midware_RPM_Wakeup_Int(void);
UCHAR Midware_RPM_Get_WakeupFlag(void);
UINT32 Midware_Timer_Get(void);
UINT16 Midware_RPM_Get(UCHAR Mode);

#endif
