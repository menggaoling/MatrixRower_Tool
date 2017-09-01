#ifndef __MIDWARE_ECB_01_H__
#define __MIDWARE_ECB_01_H__

#include "Hal_ECB_01.h"

typedef struct
{
  unsigned zeroShort : 1;  
  unsigned zeroOpen  : 1;
  unsigned noCount   : 1;             
}ECB_ERROR;

void Midware_ECB_Initial_HW(void);
void Midware_ECB_Initial_Data(void);
void Midware_ECB_Get_Error(ECB_ERROR *err);
void Midware_ECB_Set_TargetCount(UINT16 data);
void Midware_ECB_Set_Action(UCHAR data);
WORD Midware_ECB_Get_Count(void);
void Midware_ECB_1ms_Int(void);
void Midware_ECB_Process(void);
void Midware_ECB_Clr_ZeroFlag(void);
void Midware_ECB_ERP();

#endif	

