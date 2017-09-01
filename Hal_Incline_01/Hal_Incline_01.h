#ifndef __HAL_INCLINE_01_H__
#define __HAL_INCLINE_01_H__

#include "JIStypes.h"


void Hal_Incline_Initial(void);  
void Hal_Incline_Set_Stop(void);
void Hal_Incline_Set_UpDir(void);
void Hal_Incline_Set_DownDir(void);
UCHAR Hal_Incline_Check_Zero(void);
void Hal_Incline_ERP(void);
void Hal_Incline_Set_Control(UCHAR level);
void Hal_Incline_Set_Intfuct(FNCT_VOID FnctX);
void Hal_Incline_Int(void);
#endif	

