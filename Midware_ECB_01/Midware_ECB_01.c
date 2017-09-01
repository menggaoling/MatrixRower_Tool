#include "Midware_ECB_01.h"

__no_init static struct
{
  unsigned allowAct       : 1;
  unsigned zeroOK         : 1; 
  unsigned atZero         : 1;
}ECB_STS;

#define ECB_COUNT_MAX           380  
#define ECB_COUNT_OFFSET	1

#define ECB_COMMAND_STOP	0x00
#define ECB_COMMAND_UP	        0x01
#define ECB_COMMAND_DOWN	0x80

#define ECB_ZERO_SHORT_MIN_CNT  80
#define ECB_MAX_CNT_OFFSET      50

#define ECB_10MS_CIRCLE         10      
#define ECB_DELAY_ACT_TIME	(1000 / ECB_10MS_CIRCLE)      // 1S
#define ECB_BLOCK_TIME	        (5000 / ECB_10MS_CIRCLE)      // 5S
#define ECB_FORCE_ZERO_TIME     (60000 / ECB_10MS_CIRCLE)     // 60S  

#define ECB_STOP                0
#define ECB_UP                  1
#define ECB_DOWN                2

__no_init static UINT16 ECBtargetCnt, ECBcurrentCnt, ECBnoCntNum, zeroCnt;
__no_init static UCHAR ECBrunSts, zeroCheckTime, upDelay, downDelay;
__no_init static ECB_ERROR ECB_ERR; 
__no_init static UINT16 forceZeroTime;

static void Midware_ECB_Int(void)
{  
  if(ECB_STS.allowAct == 0) return;
  
  if( ECBrunSts == ECB_UP || (ECBrunSts == ECB_STOP && downDelay) )
  {
    if(ECBcurrentCnt < ECB_COUNT_MAX)
      ++ECBcurrentCnt;
    ECBnoCntNum = 0;
    ECB_ERR.noCount = 0;
  }
  else if( ECBrunSts == ECB_DOWN || (ECBrunSts == ECB_STOP && upDelay) )
  {
    if(ECBcurrentCnt > 0)
      --ECBcurrentCnt;
    if(ECB_STS.zeroOK == 0)
      ++zeroCnt;
    ECBnoCntNum = 0;
    ECB_ERR.noCount = 0;
  }	
}

void Midware_ECB_Initial_HW(void)
{
  Hal_ECB_Initial();  
  Hal_ECB_Set_KBIfuct(Midware_ECB_Int);
}

void Midware_ECB_Initial_Data(void)
{    
  ECB_STS.allowAct = 0;
  ECBcurrentCnt = 0; 
  ECBtargetCnt = 0; 
  ECB_STS.zeroOK = 0;
  ECB_STS.atZero = 0;
  ECB_ERR.noCount = 0;
  ECB_ERR.zeroShort = 0;
  ECB_ERR.zeroOpen = 0;
  ECBnoCntNum = 0;
  upDelay = 0;
  downDelay = 0;
  zeroCheckTime = 0;
  zeroCnt = 0;
  ECBrunSts = ECB_STOP;  
  forceZeroTime = 0;
  
}

void Midware_ECB_Get_Error(ECB_ERROR *err)
{
  memcpy(err, &ECB_ERR, sizeof(ECB_ERR));
}

void Midware_ECB_Set_TargetCount(UINT16 data)
{
  ECBtargetCnt = data;     
  if(ECBtargetCnt > ECB_COUNT_MAX)
    ECBtargetCnt = ECB_COUNT_MAX;   
  ECB_STS.allowAct = 1; 
}

void Midware_ECB_Set_Action(UCHAR data)
{
  if( data == ECB_COMMAND_UP )
    ECBtargetCnt =  ECB_COUNT_MAX ;
  else if( data == ECB_COMMAND_DOWN )
    ECBtargetCnt =  0 ;
  else if ( data == ECB_COMMAND_STOP )
    ECBtargetCnt = ECBcurrentCnt;
  ECB_STS.allowAct = 1; 
}

UINT16 Midware_ECB_Get_Count(void)
{		
  return ECBcurrentCnt;
}

void Midware_ECB_ZeroCheck(void)
{
  if(++zeroCheckTime % 5 == 0)
  {
    if(Hal_ECB_Check_Zero() == 0)
    {
      zeroCheckTime = 0;
      ECB_STS.atZero = 0;
    }
    else if(zeroCheckTime >= 20 && Hal_ECB_Check_Zero() > 0)
    {
      ECB_STS.atZero = 1;
      ECBcurrentCnt = 0;
      if(ECB_STS.zeroOK == 0)
      {
        ECB_STS.zeroOK = 1;
      }
    }
  }
  if(zeroCheckTime >= 20) zeroCheckTime = 0;
  if(ECB_STS.zeroOK == 1 && ECBrunSts == ECB_STOP && ECBtargetCnt == 0)
  {
    if(++forceZeroTime > ECB_FORCE_ZERO_TIME)
    {
      forceZeroTime = 0;
      if(ECB_STS.atZero == 0)
      {
        zeroCnt = 0; 
        ECB_STS.zeroOK = 0; 
      }
    }
  }
}

void Midware_ECB_1ms_Int(void)
{
  if(ECB_STS.allowAct == 0) return;
  
  static UCHAR byTimer = 0;
  if(++byTimer >= ECB_10MS_CIRCLE)  // 100ms every time;
  {
    byTimer = 0;       
    if(ECBrunSts != ECB_STOP) 
    {           
      if(++ECBnoCntNum > ECB_BLOCK_TIME)  //check whether have count
      {
        ECBnoCntNum = 0;
        ECB_ERR.noCount = 1;
      } 
      
      if(ECB_STS.zeroOK == 1)
      {    
        if(ECBcurrentCnt > ECB_ZERO_SHORT_MIN_CNT)
        {
          if(ECB_STS.atZero == 1)
            ECB_ERR.zeroShort = 1;  
        }
      }
      else
      {
        if(zeroCnt > ECB_COUNT_MAX + ECB_MAX_CNT_OFFSET)
          ECB_ERR.zeroOpen = 1;
      }
    }
    else if(ECBrunSts == ECB_STOP)
    { 
      if(upDelay > 0 ) --upDelay;
      if(downDelay > 0 ) --downDelay;      
    }          
  }  
  
  Midware_ECB_ZeroCheck();    
}

void Midware_ECB_Clr_ZeroFlag(void)
{
  zeroCnt = 0;
  ECB_STS.zeroOK = 0 ;
  ECB_STS.allowAct = 1; 
}

void Midware_ECB_Process(void)
{	 
  UCHAR needStop = 1;
  if(ECB_STS.allowAct == 1 && ECB_ERR.noCount == 0 && ECB_ERR.zeroShort == 0 && ECB_ERR.zeroOpen == 0)
  {
    if(ECB_STS.zeroOK == 0)  
    {
      ECBrunSts = ECB_DOWN;
      Hal_ECB_Set_DownDir(); 
      upDelay = ECB_DELAY_ACT_TIME ; 
      downDelay = 0;
      return;
    }
    
    if(ECBcurrentCnt + ECB_COUNT_OFFSET < ECBtargetCnt)  
    {    
      if(upDelay == 0)
      {
        needStop = 0;
        forceZeroTime = 0;
        ECBrunSts = ECB_UP;
        Hal_ECB_Set_UpDir(); 
        downDelay = ECB_DELAY_ACT_TIME;
      }
    }
    else if(ECBcurrentCnt > (ECBtargetCnt + ECB_COUNT_OFFSET)) 
    {
      if(downDelay == 0)
      {
        needStop = 0;
        forceZeroTime = 0;
        ECBrunSts = ECB_DOWN;
        Hal_ECB_Set_DownDir();
        upDelay = ECB_DELAY_ACT_TIME;
      }
    }
  }
  
  if(needStop) //stop
  {
    ECBnoCntNum = 0;
    ECBrunSts = ECB_STOP;
    Hal_ECB_Set_Stop(); 
  }
}

void Midware_ECB_ERP(void)
{
  Hal_ECB_ERP();
}
