#ifndef __HAL_LED_01__
#define __HAL_LED_01__

#include "Hal_Led_01.h"

typedef enum 
{
    LED_NORMAL                  = 1,
    LED_NO_RPM                  = 2,
    LED_INC_SHORT               = 6,
    LED_COMM_FAIL,
    LED_INC_NO_CNT,
    LED_FLASH_ERR               = 13,
    /********************LCB**************/  
    LED_ECB_ERR                 = 3,
    LED_IND_ERR                 = 4,  
    LED_LCB_UNKNOWN             = 9,
    LED_TEMP_ABNORMAL           = 10,
    /********************MCB**************/  
    LED_OVER_CURRENT            = 3,
    LED_MOS_FAULT,
    LED_SAFEKEY,
    LED_MCB_TYPE_ERROR          = 9,
    LED_MCB_TEMP_UNNORMAL,
    LED_MCB_TEMP_OVER,
    LED_SPEED_LOW,
} LED_TYPE;

void Midware_Led_Init_HW(void);
void Midware_Led_Init_Data(void);
void Midware_Led_Mode(LED_TYPE Mode);
void Midware_Led_1ms_Int(void);
void Midware_Led_ERP(void);

#endif

