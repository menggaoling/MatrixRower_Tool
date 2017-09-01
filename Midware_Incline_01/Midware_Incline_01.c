#include "Midware_Incline_01.h"

__no_init static struct
{
  unsigned allowAct            : 1;
  unsigned zeroOK              : 1;   
  unsigned selfHelp            : 1;   
  unsigned atZero              : 1;   
}INC_STS;

#define INC_COUNT_OFFSET	    4
#define INC_STOP                    0
#define INC_UP                      1
#define INC_DOWN                    2

#define OPEN_MAIN_RELAY             0
#define ACTION_MINOR_RELAY          1
#define CLOSE_MAIN_RELAY            2
#define NO_CHANGE                   3
#define RELAY_DELAY_TIME            100 //ms

#define HALF_CIRCLE_INT_CNT         1	 
#define INC_COMMAND_STOP	    0x00
#define INC_COMMAND_UP              0x01
#define INC_COMMAND_DOWN	    0xff
#define INC_MAX_COUNT_DEF	    3000
#define INC_ZERO_SHORT_MIN_CNT      200
#define INC_SLEFHELP_MIN_CNT        700
#define INC_1MS_INTERVAL	    10                            
#define INC_COUNT_MIN_INTERVAL	    7                           

#define INC_DELAY_ACT_TIME	    (1000 / INC_1MS_INTERVAL)     //1s
#define INC_RUNNING_BLOCK_TIME	    (250 / INC_1MS_INTERVAL)      //0.25S
#define INC_STARTING_BLOCK_TIME	    (4000 / INC_1MS_INTERVAL)     //4S
#define INC_SAVE_SELF_TIME	    (5000 / INC_1MS_INTERVAL)     //5S

__no_init static UINT16 incTargetCnt, incCurrentCnt, incBlockTime;
__no_init static UINT16 incNoCntNum, incMaxCnt, incCntInterval;
__no_init static UCHAR incCurrentSts, zeroCheckTime, relayTime; 
__no_init static UINT16 upDelay, downDelay;
__no_init INCLINE_ERROR INC_ERR ;

static void Mideware_Incline_Set_RunSts(UCHAR sts);

void Midware_Incline_Int(void)
{  
  if(INC_STS.allowAct == 0) return;
  if( incCntInterval >= INC_COUNT_MIN_INTERVAL )  
  {
    if( incCurrentSts == INC_UP || (incCurrentSts == INC_STOP && downDelay) )
    {
      ++incCurrentCnt;
    }
    else if( incCurrentSts == INC_DOWN || (incCurrentSts == INC_STOP && upDelay) )
    {
      if(incCurrentCnt > 0)
        --incCurrentCnt;                     
      if(incCurrentSts == INC_DOWN && INC_STS.selfHelp == 1)
        INC_STS.selfHelp = 0;
    }
    if( incCurrentSts != INC_STOP )
    {
      incBlockTime = INC_RUNNING_BLOCK_TIME;
    }
    incCntInterval = 0;
    incNoCntNum = 0;     
  }
}

void Midware_Incline_Initial_HW(void)  
{
  Hal_Incline_Initial();
  Hal_Incline_Set_Intfuct(Midware_Incline_Int);
}

void Midware_Incline_Initial_Data(void)  
{
  incCntInterval = 0;
  INC_STS.selfHelp = 0;
  INC_STS.zeroOK = 0;
  INC_STS.allowAct = 0;
  INC_STS.atZero = 0;
  INC_ERR.noCount = 0;
  INC_ERR.zeroShort = 0;  
  incCurrentCnt = 0; 
  incTargetCnt = 0; 
  upDelay = 0;
  downDelay = 0;
  incNoCntNum = 0;
  zeroCheckTime = 0;
  incMaxCnt = INC_MAX_COUNT_DEF;
  incBlockTime = INC_STARTING_BLOCK_TIME;
  incCurrentSts = INC_STOP;
  relayTime = 0;
  
}

void Midware_Incline_Get_Error(INCLINE_ERROR *by_err)
{
  memcpy(by_err, &INC_ERR, sizeof(INCLINE_ERROR));
}

void Midware_Incline_Set_TargetCount(UINT16 wData)
{
  incTargetCnt = wData;  
  if(incTargetCnt > incMaxCnt) incTargetCnt = incMaxCnt;
}

void Midware_Incline_Allow_Act(void)
{
  INC_STS.allowAct = 1 ;
}

void Midware_Incline_Set_Action(UCHAR byData)
{
  if( byData == INC_COMMAND_UP )
    incTargetCnt =  incMaxCnt ;
  else if( byData == INC_COMMAND_DOWN )
    incTargetCnt =  0 ;
  else if ( byData == INC_COMMAND_STOP )
    incTargetCnt = incCurrentCnt;
}

UINT16 Midware_Incline_Get_Count(void)
{		
  return incCurrentCnt;
}

void Midware_Incline_ZeroCheck(void)
{
  if(++zeroCheckTime % 5 == 0)
  {
    if(Hal_Incline_Check_Zero() > 0)
    {
      zeroCheckTime = 0;
      INC_STS.atZero = 0;
    }
    else if(zeroCheckTime >= 20 && Hal_Incline_Check_Zero() == 0)
    {
      INC_STS.atZero = 1;
      if(INC_ERR.noCount == 0 && INC_STS.zeroOK == 0)
      {
        INC_STS.zeroOK = 1;
        incCurrentCnt = 0;
      }
    }
  }
  if(zeroCheckTime >= 20) zeroCheckTime = 0;
}

void Midware_Incline_1ms_Int(void)
{
  if(INC_STS.allowAct == 0) return;
  
  static UCHAR byTimer = 0;
  
  if(incCurrentSts != INC_STOP || upDelay || downDelay) 
  {
    ++incCntInterval;
  }   
  
  if(++byTimer >= INC_1MS_INTERVAL)
  {
    byTimer = 0;
    
    if(incCurrentSts != INC_STOP) 
    {         			
      if(++incNoCntNum > incBlockTime)  
      {
        incNoCntNum = 0;
        INC_ERR.noCount = 1;
        if( incCurrentSts == INC_UP )
        {
          if(INC_STS.selfHelp == 0 && incCurrentCnt > INC_SLEFHELP_MIN_CNT)
          {
            incBlockTime = INC_SAVE_SELF_TIME;
            INC_STS.selfHelp = 1; 
            incTargetCnt = incCurrentCnt - 30;
            incMaxCnt = incTargetCnt;
            INC_ERR.noCount = 0;
          }
        }
      } 
      
      if( INC_STS.zeroOK == 1 )
      {               
        if(incCurrentCnt > 200 && INC_STS.atZero == 1) 
        {
          INC_ERR.zeroShort = 1;
        }
      }
    }
    else
    {
      if(upDelay > 0 ) --upDelay;
      if(downDelay > 0 ) --downDelay;
    }
  }
  if(relayTime > 0) --relayTime;
  Midware_Incline_ZeroCheck();
}

void Mideware_Incline_Set_RunSts(UCHAR sts)
{
  static UCHAR lastSts = 0;
  static UCHAR stage = 0;
  if(lastSts != sts)
  {
    lastSts = sts;
    relayTime = 0;
    stage = 0;
  }
  if(relayTime > 0) return;
  switch(stage)
  {
  case OPEN_MAIN_RELAY:
#ifdef __LCB_SYS__
    Hal_Incline_Set_Control(0);
    relayTime = RELAY_DELAY_TIME;
#endif
    break;
  case ACTION_MINOR_RELAY:
    if(sts == INC_STOP)
      Hal_Incline_Set_Stop(); 
    else if(sts == INC_UP)
      Hal_Incline_Set_UpDir();
    else if(sts == INC_DOWN)
      Hal_Incline_Set_DownDir(); 
#ifdef __LCB_SYS__
    relayTime = RELAY_DELAY_TIME;
#endif
    break;
  case CLOSE_MAIN_RELAY:
#ifdef __LCB_SYS__
    if(sts != INC_STOP)
    {
      Hal_Incline_Set_Control(1); 
    }
#endif
    incCurrentSts = sts;
    break;
  }
  if(++stage > CLOSE_MAIN_RELAY) stage = NO_CHANGE;
}

void Midware_Incline_Process(UCHAR bySafeKeyStatus)
{	  
  UCHAR needStop = 1;
  
  if(bySafeKeyStatus) INC_STS.allowAct = 0;
  if(INC_STS.allowAct == 1 && INC_ERR.zeroShort == 0 && INC_ERR.noCount == 0)
  {        
    if(INC_STS.zeroOK == 0) 
    {
      Mideware_Incline_Set_RunSts(INC_DOWN);
      upDelay = INC_DELAY_ACT_TIME ; 
      downDelay = 0;
      return;
    } 
    
    if(incCurrentCnt + INC_COUNT_OFFSET < incTargetCnt)      
    {    
      if(upDelay == 0)
      {
        Mideware_Incline_Set_RunSts(INC_UP);
        downDelay = INC_DELAY_ACT_TIME;
        needStop = 0;
      }
    }
    else if(incCurrentCnt > (incTargetCnt + INC_COUNT_OFFSET))   
    {
      if(downDelay == 0)
      {
        Mideware_Incline_Set_RunSts(INC_DOWN);
        upDelay = INC_DELAY_ACT_TIME;
        needStop = 0;
      }
    }
  }
  
  if(needStop) 
  {
    incNoCntNum = 0;
    Mideware_Incline_Set_RunSts(INC_DOWN);
    incBlockTime = INC_STARTING_BLOCK_TIME;
  }
}

void Midware_Incline_ERP(void)
{
  Hal_Incline_Initial();
}

UCHAR Midware_Incline_IsUp(void)
{
  return ((incCurrentSts == INC_UP) ? 1 : 0 );
}

UCHAR Midware_Incline_IsDown(void)
{
  return ((incCurrentSts == INC_DOWN) ? 1 : 0 );
}
