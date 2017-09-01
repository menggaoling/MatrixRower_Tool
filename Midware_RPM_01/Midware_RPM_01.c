#include "Midware_RPM_01.h"
#include "Framework_System_01.h"

#define RPM_TIME_BASE               60000           // 60000ms
#define RPM_MAX                     60000             
#define RPM_MIN                     8               
#define MIN_PERIOD                  (RPM_TIME_BASE / RPM_MAX)             
#define MAX_PERIOD                  (RPM_TIME_BASE / RPM_MIN)            
#define DECAY_MIN_TIME              1000            // 1000ms

//per pulse is 1.6 us,use 8 sensors RPM pricision 0.1
#define RPM_CONSTANT                46875000//RPM_TIME_BASE*100000 /(16*8)          

__no_init static UINT32 presentRPM;
__no_init static UINT32 signalPeriod, decayTime, accTime; 
__no_init static struct 
{
  unsigned captured          : 1;
  unsigned locked            : 1;
  unsigned wakeupFlag        : 1;
}RPM_STS;

static UINT32 RPM0,RPM1;

void Midware_RPM_Initial_HW(void)
{
  Hal_RPM_Initial();
  Hal_RPM_Set_KBIfuct(Midware_RPM_Wakeup_Int);
}

void Midware_RPM_Initial_Data(void)
{
  presentRPM = 0;
  signalPeriod  =  0;	
  RPM_STS.captured =  0;
  RPM_STS.locked =  0;
  decayTime = 0;
  RPM_STS.wakeupFlag = 0;
  RPM0 = 0;
  RPM1 = 0;
  
}


void Midware_RPM_1ms_Int(void)    
{ 
  if(decayTime > 0) 
  {
    --decayTime;
  }
  if(accTime < 0xffff) ++accTime;
  Hal_RPM_Switch();
}

void Midware_RPM_Process(void)
{ 
  UINT32 Temp = Hal_RPM_Get_Level(0);
//  UINT32 Differ; 
  if(Temp != 0)
  {
//    if(signalPeriod != Temp)
    {     
      if(RPM0 != 0 )
      {
        if(RPM1 == 0)
          RPM1 = presentRPM;
        else
          RPM1 = RPM0;
      }
      signalPeriod = Temp;
      RPM0 = RPM_CONSTANT/signalPeriod;               
    }
  }
  else //if(Hal_RPM_Get_Stop_Status() == 1)
  {
    if(Hal_RPM_Get_Stop_Status() == 1)
      presentRPM = 0;
    else
      presentRPM = RPM1;
    RPM1 = 0;
    RPM0 = 0;    
    signalPeriod = 0;      
  }
}
//{
////  Midware_RPM_Scan();
//  if(RPM_STS.captured)
//  {
//    RPM_STS.captured =  0;
//    if(signalPeriod > MAX_PERIOD) 
//    {
//      presentRPM =  0;
//    }
//    else if(signalPeriod <= MAX_PERIOD && signalPeriod >= MIN_PERIOD)
//    {
//      presentRPM = RPM_TIME_BASE / signalPeriod;
//      presentRPM /= 8;
//    }
//    else if(signalPeriod < MIN_PERIOD)
//    {
//      presentRPM = RPM_MAX;
//    }
//    if(presentRPM > 0) decayTime = RPM_TIME_BASE / presentRPM + DECAY_MIN_TIME;
//    RPM1 = RPM0;
//    Timer1 = Timer0;
//    Timer0 = MilliSeconds_Timer_Get();
//    RPM0 = presentRPM;
//  }
//  else 
//  {
//    if(decayTime == 0 && presentRPM > 0) 
//    {
//      presentRPM = 0;
//      RPM0 = 0;
//      RPM1 = 0;
//      Timer0 = 0;
//      Timer1 = 0; 
//    }     
//  }          
//}

UINT16 Midware_RPM_Get_RPM(void) 
{       
//  return (UINT16)((RPM0 + RPM1 + 1)/2);
  return (UINT16) RPM0;
//  return (UINT16) RPM1;
}

void Midware_RPM_Erp(void)
{
  Hal_RPM_ERP();
}

void Midware_RPM_Wakeup_Int(void)
{
  RPM_STS.wakeupFlag = 1;
}

UCHAR Midware_RPM_Get_WakeupFlag(void)
{
  UCHAR flag = RPM_STS.wakeupFlag;
  RPM_STS.wakeupFlag = 0;
  return flag;
}

UINT16 Midware_RPM_Get(UCHAR Mode)
{
  UINT16 RPM;
  if(Mode == 0 )
  {
    RPM = RPM0;
  }
  else
  {
    RPM = RPM1;
  }
  return RPM;
}

UINT32 Midware_Timer_Get(void)
{
  return signalPeriod;
}
