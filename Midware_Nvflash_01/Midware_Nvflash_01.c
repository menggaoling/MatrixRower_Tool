#include "Midware_Nvflash_01.h"

__no_init static NVFLASH_ERROR NVF_ERR;


static UCHAR Midware_Nvflash_FixedArea_Save_OneMass(UCHAR page)  
{
    UCHAR err = ERROR;
    if(NVF_ERR.write == 0)
    {
        err = Hal_Nvflash_FixedArea_Write_OneMass(page);  
    }
    
    if(err == ERROR) NVF_ERR.write = 1;   
    
    return err;
}

void Midware_Nvflash_Initial_Data(void)
{
    NVF_ERR.write = 0;
    NVF_ERR.read = 0;
    if(Hal_Nvflash_Initial() == ERROR)
    {
        NVF_ERR.write = 1;
    }  
}

void Midware_Nvflash_GetError(NVFLASH_ERROR *err)
{
    memcpy(err, &NVF_ERR, sizeof(NVFLASH_ERROR));
}

UCHAR Midware_Nvflash_Write_Data(UINT32 address, UCHAR *data, UINT16 length)
{
    UCHAR by_Flash_Buff[FLASH_WR_SIZE] = {0};
    memcpy(by_Flash_Buff, data, length);//solve align problem
    return Hal_Nvflash_FreeArea_Write_Data(address, by_Flash_Buff, length);
}

void Midware_Nvflash_Read_Data(UCHAR *address, UCHAR *data, UINT16 length)
{
  memcpy(data,address,length);
}

UCHAR Midware_Nvflash_Erase_Flash(UINT32 address)
{
    return Hal_Nvflash_Erase_Flash(address);
}

UCHAR Midware_Nvflash_Prepare_CalibratedDatas(UINT16 *data, UCHAR length)  
{
    UCHAR err = ERROR;   
    if(length > 0 && length <= CALIBRATE_BLOCK_SIZE)
    {
        Hal_Nvflash_FixedArea_Read_OneMass(CALIBRATE_PAGE);
        for(BYTE i = 0; i < length; i++)
        {
            Hal_Nvflash_Write_Buffer(i,data[i]);
        }
        err = RIGHT;
    }    
    if(err == ERROR) NVF_ERR.write = 1;      
    return err;
}

UCHAR Midware_Nvflash_Prepare_OneCalibratedData(UCHAR index, UINT16 data)  
{
    UCHAR err = ERROR;  
    if(index < CALIBRATE_BLOCK_SIZE)
    {
        Hal_Nvflash_FixedArea_Read_OneMass(CALIBRATE_PAGE);
        Hal_Nvflash_Write_Buffer(index ,data);
        err = RIGHT;
    }  
    if(err == ERROR) NVF_ERR.write = 1;     
    return err;
}

UCHAR Midware_Nvflash_Prepare_OneUpdatedInfor(UCHAR index, UINT16 data)  
{
    UCHAR err = ERROR;    
    if(index < UPDATE_BLOCK_SIZE)
    {
        Hal_Nvflash_FixedArea_Read_OneMass(UPDATE_PAGE);
        Hal_Nvflash_Write_Buffer(index ,data);
        err = RIGHT;
    }    
    if(err == ERROR) NVF_ERR.write = 1;      
    return err;
}

UCHAR Midware_Nvflash_Prepare_UpdatedInformations(UINT16 *data, UCHAR length)  
{
    UCHAR err = ERROR;
    if(length > 0 && length <= UPDATE_BLOCK_SIZE)
    {
        Hal_Nvflash_FixedArea_Read_OneMass(UPDATE_PAGE);
        for(BYTE i = 0; i < length ; i++)
        {
            Hal_Nvflash_Write_Buffer(i,data[i]);
        }
        err = RIGHT;
    }
    
    if(err == ERROR) NVF_ERR.write = 1;   
    
    return err;
}

UCHAR Midware_Nvflash_Save_CalibratedDatas(void)
{
   return Midware_Nvflash_FixedArea_Save_OneMass(CALIBRATE_PAGE);
}

UCHAR Midware_Nvflash_Save_UpdatedInformations(void)
{
   return Midware_Nvflash_FixedArea_Save_OneMass(UPDATE_PAGE);
}

UINT16 Midware_Nvflash_Get_OneCalibratedData(UCHAR index)
{
    UINT16 data = 0;
    
    if( index < CALIBRATE_BLOCK_SIZE && NVF_ERR.read == 0)
        data = Hal_Nvflash_FixedArea_Read_OneData(CALIBRATE_PAGE, index);
    else 
        NVF_ERR.read = 1;
    
    return data;
}

UINT16 Midware_Nvflash_Get_OneUpdatedInfor(UCHAR index)
{
    UINT16 data = 0;
    
    if( index < UPDATE_BLOCK_SIZE && NVF_ERR.read == 0)
        data = Hal_Nvflash_FixedArea_Read_OneData(UPDATE_PAGE, index);
    else 
        NVF_ERR.read = 1;
    
    return data;
}