#ifndef __HAL_MOTOR_H__
#define __HAL_MOTOR_H__

#include "jistypes.h"

void Hal_Motor_Initial(void);
void Hal_Motor_PWM_Initial(void);
void Hal_Motor_Set_PWM(UINT16 w_Dat);
void Hal_Motor_Set_Realy_Charge(UCHAR by_Enable);
void Hal_Motor_Set_Relay_Main(UCHAR by_Enable);
UCHAR Hal_Motor_Is_OverCurrent(void);
UCHAR Hal_Motor_Is_MOSD(void);
void Hal_Motor_ERP(void);


#endif

