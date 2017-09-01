#ifndef __MIDWARE_SAFEKEY_01_H__
#define __MIDWARE_SAFEKEY_01_H__

#include "Hal_Safekey_01.h"

void Midware_Safekey_Initial_HW(void);
void Midware_Safekey_Initial_Data(void);
UCHAR Midware_Safekey_IsRemove(void);
void Midware_Safekey_1ms_Int(void);
void Midware_Safekey_ERP(void);


#endif

