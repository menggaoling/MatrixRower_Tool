#ifndef __MIDWARE_RPM_02_H__
#define __MIDWARE_RPM_02_H__

#include "Hal_RPM_MCB_01.h"

#ifdef DIGITAL_MCB

#define MAX_PULSE                 23438           //min RPM 40
#define MIN_PULSE                 187             //RPM buff��С 60 000 000 /(187.5 * 0.8 * 80) = 5000(Max RPM)... x = 937500/y
#define RPM_MIN_INTERVAL	    18		        //ms  ��С RPM 40 * 80 = 3200,  60 / 3200 = 18ms

#define BUFFER_SIZE            20                   // buff��С

#else

#define MAX_PULSE                 23438           //min RPM 40
#define MIN_PULSE                 312             //RPM buff��С 60 000 000 /(3000 * 64) = 625(Max TimeInterval)... x = 1875000/y
#define RPM_MIN_INTERVAL	    2000		    // 3s 60000000/30(RPM) = 3s = 3000ms

#define BUFFER_SIZE           10                   // buff��С

#endif


void Midware_RPM_Initial_HW(void);
void Midware_RPM_Initial_Data(void);
UCHAR Midware_RPM_Get_PulseData(UINT32 *pData);
void Midware_RPM_1ms_Int(void);
UCHAR Midware_RPM_IsRunning(void);
void Midware_RPM_ERP(void);


#endif


