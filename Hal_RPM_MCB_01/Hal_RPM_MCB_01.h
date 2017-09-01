#ifndef __OSP_HW_H__
#define __OSP_HW_H__

#include "jistypes.h"


void Hal_RPM_Initial(void);
UINT32 Hal_RPM_Get_Capture(void);
void Hal_RPM_ERP(void);
void Hal_RPM_INT(void);
void Hal_RPM_Set_Intfuct(FNCT_VOID FnctX);

#endif


