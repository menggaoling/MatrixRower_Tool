#ifndef __MIDWARE_NVFLASH_01_H
#define __MIDWARE_NVFLASH_01_H

#include "Hal_Nvflash_01.h"

typedef struct
{
    unsigned write:  1;        
    unsigned read:   1;             
}NVFLASH_ERROR;
 
#define AUTO_CAL_RPM_0      1
#define AUTO_CAL_PWM_0      2
#define AUTO_CAL_RPM_1      3
#define AUTO_CAL_PWM_1      4
#define AUTO_CAL_RPM_2      5
#define AUTO_CAL_PWM_2      6
#define AUTO_CAL_RPM_3      7
#define AUTO_CAL_PWM_3      8
#define AUTO_CAL_RPM_4      9
#define AUTO_CAL_PWM_4      10
#define AUTO_CAL_RPM_5      11
#define AUTO_CAL_PWM_5      12
#define AUTO_CAL_RPM_6      13
#define AUTO_CAL_PWM_6      14
#define AUTO_CAL_RPM_7      15
#define AUTO_CAL_PWM_7      16
#define AUTO_CAL_RPM_8      17
#define AUTO_CAL_PWM_8      18
#define AUTO_CAL_RPM_9      19
#define AUTO_CAL_PWM_9      20
#define AUTO_CAL_RPM_10     21
#define AUTO_CAL_PWM_10     22
#define AUTO_CAL_RPM_11     23
#define AUTO_CAL_PWM_11     24
#define AUTO_CAL_FLAG       25
#define C_ATUTO_CAL_FLAG    0x3535
#define CRT_TYPE_200_220    26
#define CRT_PWME_200_220    27
#define CRT_TYPE_225_220    28
#define CRT_PWME_225_220    29
#define CRT_TYPE_250_220    30
#define CRT_PWME_250_220    31
#define CRT_TYPE_275_220    32
#define CRT_PWME_275_220    33
#define CRT_TYPE_300_220    34
#define CRT_PWME_300_220    35
#define CRT_TYPE_325_220    36
#define CRT_PWME_325_220    37
#define CRT_TYPE_200_110    38
#define CRT_PWME_200_110    39
#define CRT_TYPE_225_110    40
#define CRT_PWME_225_110    41
#define CRT_TYPE_250_110    42
#define CRT_PWME_250_110    43
#define CRT_TYPE_275_110    44
#define CRT_PWME_275_110    45
#define CRT_TYPE_300_110    46
#define CRT_PWME_300_110    47
#define CRT_TYPE_325_110    48
#define CRT_PWME_325_110    49
#define CRT_FLAG            50
#define C_CRT_FLAG          0x5353
#define K_FACTOR_INDEX      51 
#define B_OFFSET_INDEX      52

#define UPDATE_VER_INDEX         0
#define PACKAGE_NUM_INDEX        1 
#define UPDATE_FLAG_HIGH_INDEX   2 
#define UPDATE_FLAG_LOW_INDEX    3 

void Midware_Nvflash_Initial_Data(void);
UCHAR Midware_Nvflash_Prepare_CalibratedDatas(UINT16 *data, UCHAR length);
UCHAR Midware_Nvflash_Prepare_OneCalibratedData(UCHAR index, UINT16 data);
UCHAR Midware_Nvflash_Prepare_OneUpdatedInfor(UCHAR index, UINT16 data);
UCHAR Midware_Nvflash_Prepare_UpdatedInformations(UINT16 *data, UCHAR length);
UCHAR Midware_Nvflash_Save_CalibratedDatas(void);
UCHAR Midware_Nvflash_Save_UpdatedInformations(void);
void Midware_Nvflash_GetError(NVFLASH_ERROR *err);
UCHAR Midware_Nvflash_Write_Data(UINT32 address, UCHAR *data, UINT16 length);
void Midware_Nvflash_Read_Data(UCHAR *address, UCHAR *data, UINT16 length);
UCHAR Midware_Nvflash_Erase_Flash(UINT32 address);
UINT16 Midware_Nvflash_Get_OneCalibratedData(UCHAR index);
UINT16 Midware_Nvflash_Get_OneUpdatedInfor(UCHAR index);
#endif

