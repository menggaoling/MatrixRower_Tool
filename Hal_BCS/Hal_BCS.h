#ifndef __BCS_HW_H__
#define __BCS_HW_H__

#include "jistypes.h"

void Hal_BCS_PWM_Initial(void);
void Hal_BCS_Set_PWM_Percent(UINT16 w_Dat);
UINT16 Hal_BCS_ReadAdc(void);
void Hal_BCS_ERP(void);


#endif

