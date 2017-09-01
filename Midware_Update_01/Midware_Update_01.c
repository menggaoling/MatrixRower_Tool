#include "Midware_Update_01.h"
#include "Midware_Nvflash_01.h"
#include "Midware_Digital_01.h"
#include "Hal_System_01.h"


#define FISRT_VERSION             1
#define SECOND_VERSION            4

#define APP_UPDATE_ADDRESS        0xFE00
#define APP_SAVED_ADDRESS         0x8000
#define FLASH_END_ADDRESS         0xF7FF       //left 2k used for information
#define APP_UPDATE_FLAG           0xa5a55a5a


__no_init static UCHAR  resetFlag;
__no_init static UCHAR  programState;

void Midware_Update_Init_Data(void)
{  
  resetFlag = 0;
  programState = 0;
}

void Midware_Update_Process(UCHAR RxCmd, UCHAR *Rxdata, UINT16 RxDataLen, UCHAR *TxData, UINT16 *TxDataLen)
{ 
  *TxDataLen = 1;
  TxData[0] = 0x01;
  switch(RxCmd)
  {
  case CmdFlashUnlock://0x24
    {
      unsigned char *abyTemp = "JOHNSON"; 
      if(strncmp((char*)Rxdata, (char*)abyTemp,RxDataLen) != 0)
      {
        TxData[0] = 0;            
      }		
      break;
    }
  case CmdEraseFlash://0x10
    {
      DataUnion32 EraseAddress ;
      DataUnion32 EraseLength ;      
      TxData[0] = 0; 
      memcpy(EraseAddress.data8, Rxdata, 4);  
      memcpy(EraseLength.data8, Rxdata + 4, 4); 
      EraseLength.data32 >>= 9;//length/512
      if(EraseAddress.data32 >= APP_SAVED_ADDRESS )
      {
        for(UINT16 i = 0;i < EraseLength.data32; i++)
        {
          Hal_System_WDOGFeed();  //WATCHDOG_RESET ,or erase will not success
          if(Midware_Nvflash_Erase_Flash(EraseAddress.data32) == RIGHT)          //512Bytes Erase
          {
            TxData[0] = 1;  
          }   
          else
          {
            TxData[0] = 0;  
            break;
          }
          EraseAddress.data32 += FLASH_SECTOR_SIZE;
        }
      }
      *TxDataLen = 1;	
      break;
    }
  case CmdWriteFlash://0x12
    {  
      DataUnion32 StartAdd;
      //The start address be written
      memcpy(TxData, Rxdata, 4);  
      TxData[4] = 0; 
      TxData[5] = 0; 
      TxData[6] = 0;
      TxData[7] = 0;
      
      memcpy(StartAdd.data8, Rxdata, 4);  
      //due to buffer size limit,RxDataLen should less than FLASH_WR_SIZE + 4
      if(RxDataLen > (FLASH_WR_SIZE + 4))//datasize + startaddress 
        RxDataLen = FLASH_WR_SIZE + 4;
      
      if(StartAdd.data32 >= APP_SAVED_ADDRESS && StartAdd.data32 < FLASH_END_ADDRESS\
        && RxDataLen >= 4)
      {
        if(Midware_Nvflash_Write_Data(StartAdd.data32,&Rxdata[4], (RxDataLen - 4)) == RIGHT)
        {               
          //The length of data
          TxData[4] = (RxDataLen - 4); //LOW UCHAR
          TxData[5] = (RxDataLen - 4) >> 8; //HIGH UCHAR            
        }
      }
      TxData[8] = Midware_Digital_Get_Crc8(&Rxdata[4], (RxDataLen - 4)); //The CRC8 of program data       
      *TxDataLen = 9;	
      break;
    }
  case CmdReadFlash: //0x14
    {
      DataUnion32 StartAdd;
      DataUnion32 DataLength;
      
      memcpy(TxData, Rxdata, 4);                 
      memcpy(StartAdd.data8, Rxdata, 4);              
      memcpy(DataLength.data8, &Rxdata[4], 4);        
      Midware_Nvflash_Read_Data((UCHAR *)StartAdd.data32, (UCHAR *)(&TxData[4]), DataLength.data32);
      *TxDataLen = DataLength.data32 + 4;
      break;
    }
  case CmdWriteCheckCode: //0x26
    {      		
      if(Rxdata[15] == Midware_Digital_Get_Crc8(Rxdata, 15))
      {
        programState = 0x01;                                 //the main program had been updated
      }
      else
      {
        programState = 0xff;                                 //the main program block currently the data is wrong
      }             
      memcpy(TxData, Rxdata, 16);
      *TxDataLen = 16;
      break;
    }
  case CmdReadProgramState:  //0x20
    {
      if(programState == 0x01)                                 //the main program had been updated
      {
        DataUnion32 data; 
        data.data32 = APP_UPDATE_FLAG;
        FLASH_EraseSector((unsigned long)APP_UPDATE_ADDRESS);                                 
        if(Midware_Nvflash_Write_Data(APP_UPDATE_ADDRESS, data.data8, 4) == ERROR)        
        {
          programState = 0xff;
        }   		
        resetFlag = 1;	
      }
      
      TxData[0] = programState;
      break;
    }
  default:
    break;
  }
}

void Midware_Update_1ms_Int(void)
{
  static UINT16 wDelayReset = 0;
  if(resetFlag == 1)
  {
    if(++wDelayReset >= 1000)
      NVIC_SystemReset();      
  }
}