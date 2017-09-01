#ifndef __APID_H__
#define __APID_H__

#include "jistypes.h"
//#include "APID.h"


void APID_Initial_Data(void);
void APID_1ms_Int(void);
void APID_SetAutoCalibrate(void);
UCHAR APID_Calibrating(void);
UCHAR APID_Process(UINT16 wTargetRPM, UINT16 *pw_Out);
UCHAR APID_Get_TargetStable(void);
UINT16 APID_Get_Current_Target(void);
UINT16 APID_Get_RPM(void);


#endif


