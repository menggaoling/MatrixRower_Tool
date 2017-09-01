#ifndef __HAL_ECB_01_H__
#define __HAL_ECB_01_H__

#include "JIStypes.h"

void Hal_ECB_Initial(void);  
void Hal_ECB_Set_UpDir(void);
void Hal_ECB_Set_DownDir(void);
void Hal_ECB_Set_Stop(void);
UCHAR Hal_ECB_Check_Zero(void);
void Hal_ECB_ERP(void);
void Hal_ECB_Set_KBIfuct(FNCT_VOID FnctX);
void Hal_ECB_INT(void);
#endif