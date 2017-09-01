#ifndef __BCS_H__
#define __BCS_H__

#include "Hal_BCS.h"


typedef enum
{
    MOTOR_150_220=0,
    MOTOR_175_220,
    MOTOR_200_220,
    MOTOR_225_220,
    MOTOR_250_220,
    MOTOR_275_220,
    MOTOR_300_220,
    MOTOR_325_220,
    MOTOR_150_110,
    MOTOR_175_110,
    MOTOR_200_110,
    MOTOR_225_110,
    MOTOR_250_110,
    MOTOR_275_110,
    MOTOR_300_110,
    MOTOR_325_110,
    NUM_OF_MOTOR
} MOTOR_HP;
typedef enum
{
  GLOBAL_150_200HP_110V=0XB0,
  GLOBAL_150_200HP_220V=0XB1,
  GLOBAL_200_250HP_110V=0XB2,
  GLOBAL_200_250HP_220V=0XB3,
  GLOBAL_200_325HP_110V=0XD0,
  GLOBAL_200_325HP_220V=0XD1,
}MCB_TYPE;

void Midware_BCS_Initial_HW(void);
void Midware_BCS_Initial_Data(void);
void Midware_BCS_1ms_Int(void);
void Midware_BCS_CalibrationCurrent_Process(UINT16 wCurrentActual);
UCHAR Midware_BCS_CalibrationCurrent_IsOK(void);
void Midware_BCS_Init_MotorHP(void);
MOTOR_HP Midware_BCS_Get_MotorHP(void);
void Midware_BCS_ERP(void);
MCB_TYPE BCS_Get_MCB_Type(void);
void Midware_BCS_Set_OverCurrentLimited(MOTOR_HP MotorHp);
UINT16 Midware_BCS_Get_MaxMotorHP(void);
UINT16 Midware_BCS_Get_MinMotorHP(void);
#endif



