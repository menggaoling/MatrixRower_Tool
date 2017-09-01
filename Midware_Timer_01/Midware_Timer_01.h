#ifndef __MIDWARE_TIMER_01_H
#define __MIDWARE_TIMER_01_H

#include "JIStypes.h"

#define T_LOOP      0x80

void Midware_Timer_Initial_Data(void);
void Midware_Timer_1ms_Int(void);
void Midware_Timer_Clear(UCHAR by_Index);
UCHAR Midware_Timer_Counter(UCHAR by_Index, UINT16 w_Dat);


#endif
