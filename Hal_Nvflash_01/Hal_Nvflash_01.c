#include "Hal_Nvflash_01.h"

__no_init static UINT16 infoBuffer[CALIBRATE_BLOCK_SIZE];
__no_init static UCHAR CaliCurrentBlock, updateCurrentBlock;

UCHAR Hal_Nvflash_Initial(void)
{
    UCHAR err = RIGHT;
    signed char loop;
    
    memset(infoBuffer,0,sizeof(infoBuffer));
    
    for(loop = CALIBRATE_BLOCKS - 1; loop >= 0; loop--)
    {
        CaliCurrentBlock = loop;
        if(Hal_Nvflash_FixedArea_Read_OneData(CALIBRATE_PAGE, CALIBRATE_BLOCK_SIZE - 1) == USED_FLAG) break;
        if(loop == 0)
        {
            err = Hal_Nvflash_FixedArea_Write_OneMass(CALIBRATE_PAGE);
            break;
        }
    }
    
    for(loop = UPDATE_BLOCKS - 1; loop >= 0; loop--)
    {
        updateCurrentBlock = loop;
        if(Hal_Nvflash_FixedArea_Read_OneData(UPDATE_PAGE, UPDATE_BLOCK_SIZE - 1) == USED_FLAG) break;
        if(loop == 0)
        {
            err = Hal_Nvflash_FixedArea_Write_OneMass(UPDATE_PAGE);
            break;
        }
    } 
    return err;
}

void Hal_Nvflash_Write_Buffer(UINT16 num, UINT16 value)
{
    *(infoBuffer + num) = value;
}

void Hal_Nvflash_FixedArea_Read_OneMass(UCHAR page)
{
    UINT32 *Ptr;
    UINT32 *Dist;
    UCHAR length;
    if(page == CALIBRATE_PAGE)
    {
        Ptr = (UINT32 *)(CALIBRATE_FIRST_ADD + CaliCurrentBlock * CALIBRATE_BLOCK_SIZE * 2);
        length = CALIBRATE_BLOCK_SIZE;
    }
    else if(page == UPDATE_PAGE)
    {
        Ptr = (UINT32 *)(UPDATE_FIRST_ADD + updateCurrentBlock * UPDATE_BLOCK_SIZE * 2);
        length = UPDATE_BLOCK_SIZE;
    }
    
    Dist = (UINT32 *)infoBuffer; //change to a data of four bytes
    
    for(UCHAR loop = 0; loop < length / 2; loop++)
    {
        *(Dist + loop) = *(Ptr + loop);
    }
}

UCHAR Hal_Nvflash_FixedArea_Write_OneMass(UCHAR page)
{
    UCHAR err = RIGHT;
    UINT32 currentAdd;
    UCHAR length;
    
    DisableInterrupts;  
    if(page == CALIBRATE_PAGE)
    {
        ++CaliCurrentBlock ;
        currentAdd = CALIBRATE_FIRST_ADD + CaliCurrentBlock * CALIBRATE_BLOCK_SIZE * 2;
        length = CALIBRATE_BLOCK_SIZE;
        infoBuffer[CALIBRATE_BLOCK_SIZE - 1] = USED_FLAG;
        
        if(CaliCurrentBlock % (FLASH_SECTOR_SIZE / 2 / CALIBRATE_BLOCK_SIZE) == 0) //when one 521k is used fully, must erase it!!
        {
            UINT32 add = CALIBRATE_FIRST_ADD + (CaliCurrentBlock - FLASH_SECTOR_SIZE / 2 / CALIBRATE_BLOCK_SIZE) * CALIBRATE_BLOCK_SIZE * 2;     
            if(FLASH_EraseSector(add) != 0)
            {
                err = ERROR;
            }
        }
        
        if(CaliCurrentBlock >= CALIBRATE_BLOCKS)  
        {
            currentAdd = CALIBRATE_FIRST_ADD;
            CaliCurrentBlock = 0;
        }
    }
    else if(page == UPDATE_PAGE)
    {
        ++updateCurrentBlock ;
        currentAdd = UPDATE_FIRST_ADD + updateCurrentBlock * UPDATE_BLOCK_SIZE * 2;
        length = UPDATE_BLOCK_SIZE;
        infoBuffer[UPDATE_BLOCK_SIZE - 1] = USED_FLAG;
        
        if(updateCurrentBlock % (FLASH_SECTOR_SIZE / 2 / UPDATE_BLOCK_SIZE) == 0) //when one 521k is used fully, must erase it!!
        {
            UINT32 add = UPDATE_FIRST_ADD + (updateCurrentBlock - FLASH_SECTOR_SIZE / 2 / UPDATE_BLOCK_SIZE) * UPDATE_BLOCK_SIZE * 2;     
            if(FLASH_EraseSector(add) != 0)
            {
                err = ERROR;
            }
        }
        
        if(updateCurrentBlock >= UPDATE_BLOCKS)  
        {
            currentAdd = UPDATE_FIRST_ADD;
            updateCurrentBlock = 0;
        }    
    }
    if( err == RIGHT )
    {
        if(FLASH_Program((uint32_t)currentAdd, (uint32_t *)infoBuffer, (UINT16)(length * 2)) != 0)
        {
            err = ERROR;
        }
    }
    EnableInterrupts;
    return err;
}

UINT16 Hal_Nvflash_FixedArea_Read_OneData(UCHAR page, UINT16 index)
{
    UINT32 *ptr;
    UINT32 offset;
    UINT32 flag;
    UINT16 out;
    DataUnion32 target;
    
    flag = index % 2;
    offset = (index * 2) / 4;
    if(page == CALIBRATE_PAGE)
    {
        ptr = (UINT32 *)(CALIBRATE_FIRST_ADD + CaliCurrentBlock * CALIBRATE_BLOCK_SIZE * 2);
    }
    else if(page == UPDATE_PAGE)
    {
        ptr = (UINT32 *)(UPDATE_FIRST_ADD + updateCurrentBlock * UPDATE_BLOCK_SIZE * 2);
    }
    target.data32 = *(ptr + offset); //read four bytes
    if(!flag)
        out = target.data8[0];
    else 
        out = target.data8[1];
    return out;
}

UCHAR Hal_Nvflash_FreeArea_Write_Data(UINT32 address, UCHAR *data, UINT16 length)
{
    __disable_irq();
    if(FLASH_Program(address, (UINT32 *)data, (UINT32)length) != FLASH_ERR_SUCCESS)
    {
        __enable_irq();
        return ERROR;
    }
    __enable_irq();
    return RIGHT;
}

UCHAR Hal_Nvflash_Erase_Flash(UINT32 address)
{
    __disable_irq();
    if(FLASH_EraseSector(address) != FLASH_ERR_SUCCESS)
    {
        __enable_irq();
        return ERROR;
    }
    __enable_irq();
    return RIGHT;
}