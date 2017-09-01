#ifndef __HAL_INTER_01_H
#define __HAL_INTER_01_H

#include "JIStypes.h"

#define PWM_MAX                    2500

void Hal_Induction_Init(void);
void Hal_Induction_Set_PWM(UINT16 wDat);
UINT16 Hal_Induction_Collect_Current(void);
void Hal_Induction_ERP(void);

#endif