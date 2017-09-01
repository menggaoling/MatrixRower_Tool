#ifndef _SYSTEM_HW_H_
#define _SYSTEM_HW_H_

#include "jistypes.h"

#define BUS_CLK         BUS_CLK_HZ


void Hal_System_Initial(void);
void Hal_System_ERP(void);
void Hal_System_EnableRTC(void);
void Hal_System_WDOG_Feed(void);
void Hal_System_Empty_IO(void);
#endif



