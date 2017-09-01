#ifndef __HAL_MACHINE_TYPE_01_H
#define __HAL_MACHINE_TYPE_01_H

#include "JIStypes.h"

void Hal_MachineType_Init(void);
UINT16 Hal_MachineType_Collect_ADC(void);
void Hal_MachineType_End_Collect(void);

#endif