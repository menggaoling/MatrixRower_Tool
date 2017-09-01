#ifndef __MIDWARE_INCLINE_01_H__
#define __MIDWARE_INCLINE_01_H__

#include "Hal_Incline_01.h"

typedef struct
{
  unsigned zeroShort : 1;        
  unsigned noCount   : 1;             
}INCLINE_ERROR;

void Midware_Incline_Initial_HW(void);
void Midware_Incline_Initial_Data(void);
void Midware_Incline_Get_Error(INCLINE_ERROR *err);
void Midware_Incline_Set_TargetCount(UINT16 data);
void Midware_Incline_Set_Action(UCHAR data);
UINT16 Midware_Incline_Get_Count(void);
void Midware_Incline_1ms_Int(void);
void Midware_Incline_Process(UCHAR SafeKey);
void Midware_Incline_ERP(void);
void Midware_Incline_Allow_Act(void);
UCHAR Midware_Incline_IsUp(void);
UCHAR Midware_Incline_IsDown(void);
#endif	

