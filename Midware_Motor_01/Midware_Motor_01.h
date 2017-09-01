#ifndef __MIDWARE_MOTOR_01_H__
#define __MIDWARE_MOTOR_01_H__

#include "jistypes.h"

typedef struct
{
    unsigned Motor_FastSpeed    : 1;            //过速
    unsigned Motor_SlowSpeed    : 1;            //掉速
    unsigned Motor_OverCurrent  : 1;            //过流
    unsigned Motor_MOS          : 1;            //暴冲
    unsigned Motor_NoRpm        : 1;            //无RPM
    unsigned Motor_HP           : 1;            //马力
} MOTOR_ERROR;


void Midware_Motor_Initial_HW(void);
void Midware_Motor_Initial_Data(void);
void Midware_Motor_1ms_Int(void);
void Midware_Motor_SetAutoCalibrate(void);
UCHAR Midware_Motor_GetAutoCalibrate(void);
void Midware_Motor_Process(UINT16 wTarget, UCHAR Safekey);
void Midware_Motor_Init_MotorHP(void);
UINT16 Midware_Motor_Get_Output(void);
UINT16 Midware_Motor_Get_RPM(void);
UINT16 Midware_Motor_Get_RPM_Roller(void);
void Midware_Motor_Get_Error(MOTOR_ERROR *stError);
UCHAR Midware_Motor_IsRunning(void);
UCHAR Midware_Motor_RPM_IsRunning(void);
UCHAR Midware_Motor_GetIsInUsed(void);
UCHAR Midware_Motor_GetStep(void);
void Midware_Motor_ERP(void);
void Midware_Motor_Set_MotorHP(UINT16 MotorHp);

#endif





