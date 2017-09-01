#ifndef __MIDWARE_PID_01_H__
#define __MIDWARE_PID_01_H__

#include "jistypes.h"

#include "Midware_BCS.h"


void Midware_PID_Initial_Data(void);
void Midware_PID_Set_MotorHP(MOTOR_HP MotorHp);
UCHAR Midware_PID_Process(UINT16 wTargetRPM, UINT16 *pw_Out);
UCHAR Midware_PID_Get_TargetStable(void);
UINT16 Midware_PID_Get_Current_Target(void);
UINT16 Midware_PID_Get_RPM_Roller(void);
UINT16 Midware_PID_Get_RPM(void);
UINT32 Midware_PID_GetStep(void);
UCHAR Midware_PID_GetIsInUsed(void);
void Midware_PID_1ms_Int(void);


#endif

