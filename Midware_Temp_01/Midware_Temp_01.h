#ifndef __MIDWARE_TEMP_01_H__
#define __MIDWARE_TEMP_01_H__


#include "jistypes.h"




typedef enum
{
    TEMP_MCB = 0,
    TEMP_MOTOR,
    TEMP_MOS,
    TEMP_CPU,
    TEMP_NUM,
} TEMP_TYPE;

typedef struct
{
    unsigned TempMcbUnnormal    : 1;            //MCB稳定传感器异常
    unsigned TempMcb            : 1;            //Mcb温度错误
    unsigned TempMotor          : 1;            //Motor温度错误
    unsigned TempMos            : 1;            //MOS管温度错误
    unsigned TempCpu            : 1;            //CPU温度错误
} TEMP_ERROR;

void Midware_Temp_Initial_HW(void);
void Midware_Temp_Initial_Data(void);
void Midware_Temp_Process(void);
void Midware_Temp_1ms_Int(void);
UINT16 Midware_Temp_Get_Type(TEMP_TYPE Type);
void Midware_Temp_Get_Error(TEMP_ERROR *stError);


#endif


