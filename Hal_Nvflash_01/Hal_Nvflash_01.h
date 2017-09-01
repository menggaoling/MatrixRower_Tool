#ifndef __HAL_NVFLASH_01_H__
#define __HAL_NVFLASH_01_H__

#include "JIStypes.h"

#define CALIBRATE_FIRST_ADD          (125 * FLASH_SECTOR_SIZE)                 //62.5k
#define CALIBRATE_SIZE               (FLASH_SECTOR_SIZE * 2)
#define CALIBRATE_BLOCKS             8                           
#define CALIBRATE_BLOCK_SIZE         (CALIBRATE_SIZE / 2 /CALIBRATE_BLOCKS)
#define CALIBRATE_PAGE               0                            

#define UPDATE_FIRST_ADD             (125 * FLASH_SECTOR_SIZE + CALIBRATE_SIZE)     //62.5k
#define UPDATE_SIZE                  (FLASH_SECTOR_SIZE * 1)
#define UPDATE_BLOCKS                42                          
#define UPDATE_BLOCK_SIZE            (UPDATE_SIZE / 2 /UPDATE_BLOCKS)
#define UPDATE_PAGE                  1 

#define USED_FLAG                    0xa5a5
#define FLASH_WR_SIZE                256

#define ERROR                        0
#define RIGHT                        1

UCHAR Hal_Nvflash_Initial(void);
void Hal_Nvflash_Write_Buffer(UINT16 num, UINT16 value);
void Hal_Nvflash_FixedArea_Read_OneMass(UCHAR page);
UCHAR Hal_Nvflash_FixedArea_Write_OneMass(UCHAR page);
UINT16 Hal_Nvflash_FixedArea_Read_OneData(UCHAR page, UINT16 index);
UCHAR Hal_Nvflash_FreeArea_Write_Data(UINT32 address, UCHAR *data, UINT16 length);
UCHAR Hal_Nvflash_Erase_Flash(UINT32 address);

#endif