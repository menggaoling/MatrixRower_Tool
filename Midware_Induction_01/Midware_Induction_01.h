#ifndef __MIDWARE_INDUCTION_01_H__
#define __MIDWARE_INDUCTION_01_H__

#include "Hal_Induction_01.h"

typedef struct
{
  unsigned openCircuit   : 1;        
  unsigned shortCircuit  : 1;   
}INDUCTION_ERROR;

void Midware_Induction_Init_HW(void);
void Midware_Induction_Init_Data(UINT16 k, UINT16 b);
void Midware_Induction_1ms_Int(void);
void Midware_Induction_SET_TargetCurrentA(UINT16 wData);
void Midware_Induction_Process(void);
void Midware_Induction_ERP(void);
void Midware_Induction_Set_PWM(UINT16 data);
void Midware_Induction_Finish_Calibrate(UCHAR point);
void Midware_Induction_Set_BaseCurrent(UCHAR point, UINT16 current);
void Midware_Induction_Get_Error(INDUCTION_ERROR *err);
UINT16 Midware_Induction_Get_Kdata(void);
UINT16 Midware_Induction_Get_Bdata(void);

#endif