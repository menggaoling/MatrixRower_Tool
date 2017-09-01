#ifndef __HAL_RPM_01_H__
#define __HAL_RPM_01_H__

#define RPM_BUF_SIZE            200                   // buff¥Û–°
#define RPM_SAMPLE_NUM          4                   // sample number must bigger than 2

#include "JIStypes.h"

void Hal_RPM_Initial(void);
void Hal_RPM_ERP(void);
UINT32 Hal_RPM_Get_Level(UCHAR level);
void Hal_RPM_Set_KBIfuct(FNCT_VOID FnctX);
void Hal_RPM_INT(void);
void Hal_RPM_INT0(void);
void Hal_RPM_Switch(void);
UCHAR Hal_RPM_Rotate_Pulse_Num(void);
UCHAR Hal_RPM_Get_Stop_Status(void);
#endif