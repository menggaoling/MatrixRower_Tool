#include "Midware_Induction_01.h"

#define ADC_NUM                     100
#define CURRENT_INIT                400    
#define PWM_ADJUST_CIRCLE           100    //MS
#define CURRENT_PROTECT_TIME        30     //40 * PWM_ADJUST_CIRCLE = 4S;
#define K_MIN                       800
#define K_MAX                       2400
#define K_DEF                       1600
#define UCB_CURRENT_MAX             2500
#define PWM_MIN                     300

#define CURRENT_MAX_BORDER0         1100
#define CURRENT_MAX_BORDER1         1700
#define CURRENT_MIN_BORDER0         300
#define CURRENT_MIN_BORDER1         900
#define CURRENT_MAX_DEF             1216
#define CURRENT_MIN_DEF             517

__no_init static struct
{
  unsigned allowAct                 : 1;   
  unsigned adjustPWMflag            : 1;  
  unsigned offsetFlg                : 1;
  unsigned enterAdjust              : 1;
  unsigned finishCollect            : 1;
}IND_STS;

__no_init static UINT16 IndPresentPWM;
__no_init static UINT16 currentPresent, currentPresentAdc, currentTarget, currentOffset;
__no_init static INDUCTION_ERROR IND_ERR;
__no_init UCHAR adjustPWMtime, waitCollectADCtime, protectTime;
__no_init UINT16 calibratedADC1, calibratedADC2, calibratedPWM1, calibratedPWM2, calibratingPWM, kData, bData;
__no_init UINT16 currentMax, currentMin;
__no_init static UINT16 currentADC[ADC_NUM], currentFilterADC;

UINT16 Midware_Induction_Get_MidValue( UINT16 *value, UCHAR cnt )
{
  UINT16 ADCtemp = 0;
  UCHAR needSort = 0;
  UINT32 sum = 0;
  
  if(cnt > 0)
  {
    for(int i = 0; i < (cnt - 1); i++) 
    {
      for(int j = 0; j < (cnt - 1 - i); j++)
      {
        if(value[j] > value[j + 1]) 
        {
          ADCtemp = value[j];
          value[j] = value[j + 1];
          value[j + 1] = ADCtemp;
          needSort = 1;
        }
      }
      if(needSort == 0)
        break;
      needSort = 0;
    }
    
    for(int i = 20; i < cnt - 20; i++) 
      sum += value[i];
    return sum / (cnt - 40);
  }
  return 0;
}

void Midware_Induction_ADC_Collect(void) 
{
  static UCHAR currentCnt = 0;
  
  if(IND_STS.finishCollect == 1) return;
  currentADC[currentCnt] = Hal_Induction_Collect_Current();
  if(++currentCnt >= ADC_NUM) 
  {
    currentCnt = 0;
    currentFilterADC = Midware_Induction_Get_MidValue(currentADC, ADC_NUM);
    IND_STS.finishCollect = 1;
  }
}

void Midware_Induction_Init_HW(void)
{
  Hal_Induction_Init();
}

void Midware_Induction_Init_Data(UINT16 k, UINT16 b)
{
  IndPresentPWM = 0;  
  kData = k;
  bData = b;
  
  if(k < K_MIN || k > K_MAX)
    kData = K_DEF;
  if(b == 0xFFFF)
    bData = 0;
  
  protectTime = 0;
  currentPresentAdc = 0;
  currentOffset = 0;
  currentTarget = CURRENT_INIT;
  adjustPWMtime = 0;
  IND_STS.allowAct = 0;
  IND_STS.offsetFlg = 0;
  IND_STS.adjustPWMflag = 0;
  IND_STS.enterAdjust = 0;
  IND_STS.finishCollect = 0;
  currentPresent = 0;
  calibratingPWM = 0;
  calibratedADC1 = 0;
  calibratedADC2 = 0;
  calibratedPWM1 = 0;
  calibratedPWM2 = 0;
  memset(&IND_ERR, 0, sizeof(IND_ERR));
  waitCollectADCtime = 0;
  currentMax = CURRENT_MAX_DEF;
  currentMin = CURRENT_MIN_DEF;   
  memset(currentADC, 0, sizeof(currentADC)); 
  currentFilterADC = 0;
}

void Midware_Induction_1ms_Int(void) 
{
  if(IND_STS.allowAct == 0) return;
  if((IND_STS.adjustPWMflag == 0) && (++adjustPWMtime >= PWM_ADJUST_CIRCLE)) 
  {
    adjustPWMtime = 0;   
    IND_STS.adjustPWMflag = 1;
  }    
  if(waitCollectADCtime > 0) --waitCollectADCtime;
}

void Midware_Induction_SET_TargetCurrentA(UINT16 data)
{ 
  if(data < UCB_CURRENT_MAX)
    currentTarget = data;
  else currentTarget = UCB_CURRENT_MAX;
  
  IND_STS.allowAct = 1;
}

void Midware_Induction_Adjust_PWM(void)
{   
  UINT16 currentErr = 0;
  UCHAR  pwmStep = 3;  //0.12%
  UCHAR  over = 0;
  UCHAR  under = 0;
  UINT16 presentADC = 0; 
  
  if( IND_ERR.openCircuit || IND_ERR.shortCircuit ) 
  {
    IndPresentPWM = 0;
    Hal_Induction_Set_PWM( PWM_MAX );
    return;
  }    
  
  if(currentPresentAdc < currentOffset) currentPresentAdc = currentOffset;
  presentADC = currentPresentAdc - currentOffset; 
  currentPresent = 0;
  if(IndPresentPWM >= PWM_MIN)
  {
    if(bData <= 32767)
      currentPresent = (UINT32)presentADC * 100000 / ((UINT32)kData * IndPresentPWM * 100 / PWM_MAX + bData * 10);
    else 
      currentPresent = (UINT32)presentADC * 100000 / ((UINT32)kData * IndPresentPWM * 100 / PWM_MAX - (bData & 0x7FFF) * 10);
  }
  
  if(++protectTime >= CURRENT_PROTECT_TIME)
  {
    protectTime = 0;
    if( IndPresentPWM == PWM_MAX && currentPresent < 900 )
      IND_ERR.openCircuit = 1;
    else if( IndPresentPWM == PWM_MIN && currentPresent > 900 )
      IND_ERR.shortCircuit = 1;    
  }  
  
  if( currentPresent > currentTarget )
  {
    currentErr = currentPresent - currentTarget;
    over = 1;
  }
  else if( currentPresent < currentTarget )
  {
    currentErr = currentTarget  - currentPresent;
    under = 1;
  }
  
  if( currentErr > 300 )
    pwmStep = 75;    //3%
  else if( currentErr > 200 )
    pwmStep = 50;    //2%
  else if( currentErr > 100 )
    pwmStep = 25;    //1%
  else if( currentErr > 50 )
    pwmStep = 12;    //0.5%
  else if( currentErr > 25 )
    pwmStep = 6;     //0.24%
  else 
    protectTime = 0;  
  
  if(under == 1)
  {
    IndPresentPWM += pwmStep;
    if(IndPresentPWM > PWM_MAX)
      IndPresentPWM = PWM_MAX;
  }
  else if(over == 1)
  {
    if(IndPresentPWM >= pwmStep)
      IndPresentPWM -= pwmStep;
    else IndPresentPWM = 0;
  }
  
  if(IndPresentPWM <= PWM_MIN) IndPresentPWM = PWM_MIN;
  Hal_Induction_Set_PWM( PWM_MAX - IndPresentPWM );   
}


void Midware_Induction_Process(void)
{
  if(waitCollectADCtime == 0)
  {
    Midware_Induction_ADC_Collect();
  } 
  if(IND_STS.adjustPWMflag > 0 && IND_STS.finishCollect > 0)
  {
    IND_STS.finishCollect = 0;
    IND_STS.adjustPWMflag = 0;
    waitCollectADCtime = 10;
    currentPresentAdc = currentFilterADC; 
    if(IND_STS.enterAdjust == 1 && IND_STS.offsetFlg == 1)
    {
      IndPresentPWM = (UINT16)calibratingPWM * 25 / 10;
      if(IndPresentPWM > PWM_MAX) 
        IndPresentPWM = PWM_MAX;
      Hal_Induction_Set_PWM( PWM_MAX - IndPresentPWM );
    }
    else if(IND_STS.offsetFlg == 1 && IND_STS.allowAct == 1)
    {
      Midware_Induction_Adjust_PWM();	
    }
    else if(IND_STS.offsetFlg == 0)
    {
      IND_STS.offsetFlg = 1;  
      currentOffset = currentPresentAdc;
      IndPresentPWM = 400;
      Hal_Induction_Set_PWM( PWM_MAX - IndPresentPWM );
    }  
  }  
}

void Midware_Induction_ERP(void)
{
  currentTarget = 0;
  IndPresentPWM = 0;
  Hal_Induction_ERP();  
}

void Midware_Induction_Set_PWM(UINT16 i)
{
  if(i > 1000) return;
  calibratingPWM = i;
  IND_STS.enterAdjust = 1;
}

void Midware_Induction_Set_BaseCurrent(UCHAR byPoint, UINT16 wBaseCurrent)
{
  if(byPoint == 0 && wBaseCurrent >= CURRENT_MIN_BORDER0 && wBaseCurrent <= CURRENT_MIN_BORDER1) 
    currentMin = wBaseCurrent;
  else if(byPoint == 1 && wBaseCurrent >= CURRENT_MAX_BORDER0 && wBaseCurrent <= CURRENT_MAX_BORDER1) 
    currentMax = wBaseCurrent;
}

void Midware_Induction_Finish_Calibrate(UCHAR i)
{
  if(i == 0)
  {
    calibratedADC1 = currentPresentAdc - currentOffset;
    calibratedPWM1 = calibratingPWM;
  }
  else if(i == 1)
  {
    calibratedADC2 = currentPresentAdc - currentOffset;
    calibratedPWM2 = calibratingPWM;    
    UINT32 ADCdivCurrent2 = (UINT32)calibratedADC2 * 1000000 / currentMax;
    UINT32 ADCdivCurrent1 = (UINT32)calibratedADC1 * 1000000 / currentMin;
    if(ADCdivCurrent2 > ADCdivCurrent1 && calibratedPWM2 > calibratedPWM1)
    {
      kData = (ADCdivCurrent2 - ADCdivCurrent1) / (calibratedPWM2 - calibratedPWM1);
      if(ADCdivCurrent1 >= (UINT32)kData * calibratedPWM1)
      {
        bData = ADCdivCurrent1 / 100 - (UINT32)kData * calibratedPWM1 / 100;
      }
      else
      {
        bData = (UINT32)kData * calibratedPWM1 / 100 - ADCdivCurrent1 / 100;
        bData |= 0x8000;
      }
    }
    IND_STS.enterAdjust = 0;
  }
}

void Midware_Induction_Get_Error(INDUCTION_ERROR *err)
{
  memcpy(err, &IND_ERR, sizeof(INDUCTION_ERROR));
}

UINT16 Midware_Induction_Get_Kdata(void)
{
  return kData;
}

UINT16 Midware_Induction_Get_Bdata(void)
{
  return bData;
}


