#ifndef __SAFEKEY_HW_H__
#define __SAFEKEY_HW_H__

#include "jistypes.h"

void Hal_Safekey_Initial(void);
UCHAR Hal_Safekey_Get_Safekey(void);
UCHAR Hal_Safekey_Get_Shut(void);
void Hal_Safekey_ERP(void);


#endif


