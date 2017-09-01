#ifndef _HAL_SYSTEM_01_H_
#define _HAL_SYSTEM_01_H_

#include "Midware_Incline_01.h"
#include "Midware_Led_01.h"
#include "Midware_Induction_01.h"
#include "Midware_Digital_01.h"
#include "Midware_RPM_01.h"
#include "Midware_ECB_01.h"
#include "Midware_Timer_01.h"
#include "Midware_Update_01.h"

#define NMI_DISABLE()               (SIM->SOPT0 &= ~SIM_SOPT0_NMIE_MASK)
#define RTC_CLEAR_FLAG()            RTC_ClrFlags()

void Hal_System_Initial(void);
void Hal_System_ERP(void);
void Hal_System_EnableRTC(void);
void Hal_System_WDOGFeed(void);

#endif 