#include <intrinsics.h>
#include "Midware_BCS.h"
#include "Hal_BCS.h"
#include "Midware_Trace_01.h"
#include "Midware_Nvflash_01.h"
#include "Hal_Motor_01.h"
#include "math.h"

typedef struct
{
    MOTOR_HP    MotorHP;        //下控机种名
    UINT16      wPwmPercent;    //PWM
    UINT16      wCurrent;       //电流值,校验时用
} A_TYPE_TABLE;

typedef struct
{
    MCB_TYPE    MCBType; 
    MOTOR_HP	DefaultMotorHP;
    UINT16      wVotageMin;
    UINT16      wVotageMax;
    UINT16      MinMotorHP;
    UINT16      MaxMotorHP;    
} MCB_TYPE_VOTAGE;
static const MCB_TYPE_VOTAGE aby_McbTypeTable[6]=
{
  //MCBType,	            	DefaultMotorHP,	wVotageMin,      wVotageMax,   MinMotorHP,MaxMotorHP
  GLOBAL_150_200HP_110V,	MOTOR_150_110,	450,	                500,    150,       200,     
  GLOBAL_150_200HP_220V,        MOTOR_150_220,	230,			300,    150,       200,     
  GLOBAL_200_250HP_110V,	MOTOR_200_110,	320,			370,    200,       250,     
  GLOBAL_200_250HP_220V,	MOTOR_200_220,	60,			110,    200,       250,
  GLOBAL_200_325HP_110V,	MOTOR_200_110,	120,			180,    200,       325,     
  GLOBAL_200_325HP_220V,	MOTOR_200_220,	0,			30,     200,       325,     
};
#ifdef DIGITAL_MCB

#define DEFAULT_MCB_TYPE    GLOBAL_200_325HP_220V
#define DEFAULT_MOTOR_HP    MOTOR_325_220
#define DEFAULT_MAX_SIZE    14
#define MAX_MOTOR_HP    325
#define MIN_MOTOR_HP    200
static const A_TYPE_TABLE aby_TypeTable[DEFAULT_MAX_SIZE] =
{
    //MotorHP,         wPwmPercent wCurrent
    MOTOR_200_220,      470,         135,
    MOTOR_225_220,      570,         140,
    MOTOR_250_220,      660,         156,
    MOTOR_275_220,      740,         171,
    MOTOR_300_220,      790,         186,
    MOTOR_325_220,      900,         200,
    MOTOR_200_110,      550,         250,
    MOTOR_225_110,      570,         280,
    MOTOR_250_110,      660,         312,
    MOTOR_275_110,      740,         342,
    MOTOR_300_110,      790,         372,
    MOTOR_325_110,      900,         400,
};

#else
#define DEFAULT_MCB_TYPE    	GLOBAL_150_200HP_220V
#define DEFAULT_MOTOR_HP        MOTOR_200_220
#define DEFAULT_MAX_SIZE    6
#define MAX_MOTOR_HP    200
#define MIN_MOTOR_HP    150
static const A_TYPE_TABLE aby_TypeTable[DEFAULT_MAX_SIZE] =
{
    //MotorHP,         wPwmPercent
    MOTOR_150_220,      690,            0,
    MOTOR_175_220,      850,            0,
    MOTOR_200_220,      1000,           0,
    
    MOTOR_150_110,      750,            0,
    MOTOR_175_110,      880,            0,
    MOTOR_200_110,      1000,           0,
};

#endif

__no_init static MOTOR_HP Motor_Hp;
__no_init static MCB_TYPE MCB_Type;
__no_init static UINT16 MaxMotorHP,MinMotorHP;
__no_init static UCHAR by_Error;
__no_init static UCHAR by_Flag1ms;
__no_init static UCHAR by_CalFinishFlag;
__no_init static UCHAR by_StableCounter;
__no_init static UCHAR by_CurrentIndex;
__no_init static float f_CalOutput;
__no_init static UINT16 w_CalPWM;
__no_init static UINT16 w_OutputPWM;
__no_init static A_TYPE_TABLE TypeTable[DEFAULT_MAX_SIZE];


MCB_TYPE Midware_BCS_Cal_Type(void);
UINT16 Midware_BCS_Calibrate_Tune(UINT16 Target, UINT16 Actual);


void Midware_BCS_Initial_HW(void)
{
    Hal_BCS_PWM_Initial();
}

void Midware_BCS_Initial_Data(void)
{
    by_Error = 0;
    by_Flag1ms = 0;
    by_CalFinishFlag = 0;
    by_StableCounter = 0;
    f_CalOutput = 0.0;
    w_CalPWM = 0;
    w_OutputPWM = 0;
    by_CurrentIndex = 0;
    Motor_Hp = DEFAULT_MOTOR_HP;
    MCB_Type = DEFAULT_MCB_TYPE;
    MaxMotorHP=MAX_MOTOR_HP;
    MinMotorHP=MIN_MOTOR_HP;
    
    if(Midware_Nvflash_Get_OneCalibratedData( CRT_FLAG) == C_CRT_FLAG)
    {
     for(UINT8 i=0;i<12;i++)
      {
        TypeTable[i].MotorHP = (MOTOR_HP)Midware_Nvflash_Get_OneCalibratedData( CRT_TYPE_200_220+i*2); 
        TypeTable[i].wPwmPercent = Midware_Nvflash_Get_OneCalibratedData( CRT_PWME_200_220+i*2);
      }
    }
    else
    {
        memcpy(TypeTable, aby_TypeTable, sizeof(aby_TypeTable));
    }
}

void Midware_BCS_CalibrationCurrent_Process(UINT16 wCurrentActual)
{
    if(!by_CalFinishFlag)
    {
        static UCHAR by_Tick = 0;
        
        if(by_Flag1ms)
        {
          by_Flag1ms=0;
            if(w_OutputPWM < 4800)
            {
                Hal_Motor_Set_PWM((++w_OutputPWM)>>1);                           //先将马达PWM输出到最大
                by_Tick = 0;
            }
            else if(++by_Tick > 50)                                             //由于等ATE发送下来中间会有200ms左右延迟,故每50ms处理一次
            {
                by_Tick = 0;
                if(by_CurrentIndex > DEFAULT_MAX_SIZE) by_CurrentIndex = DEFAULT_MAX_SIZE;
                
                float fTemp = wCurrentActual - TypeTable[by_CurrentIndex].wCurrent;
                if(fabs(fTemp) < 30)                                            //目标值达到范围内
                {
                    if(++by_StableCounter > 10)                                 //目标值达到范围内,前持续10次
                    {
                        by_StableCounter = 0;
                        //保存数据
                        
                        TypeTable[by_CurrentIndex].wPwmPercent = w_CalPWM;
                        if(by_CurrentIndex < (DEFAULT_MAX_SIZE-1))
                        {
                            by_CurrentIndex++;
                            w_CalPWM = TypeTable[by_CurrentIndex].wPwmPercent;  //加快校验速度
                        }
                        else
                        {
                            TraceW("CRT Calibrate Save \n");
                            for(UINT8 i=0;i<12;i++)
                            {
                              Midware_Nvflash_Prepare_OneCalibratedData( CRT_TYPE_200_220+i*2, TypeTable[i].MotorHP); 
                              Midware_Nvflash_Prepare_OneCalibratedData( CRT_PWME_200_220+i*2, TypeTable[i].wPwmPercent);
                            }
                            Midware_Nvflash_Prepare_OneCalibratedData( CRT_FLAG, C_CRT_FLAG);
                            Midware_Nvflash_Save_CalibratedDatas();
                            //打印校验数据值
                            for(UCHAR i=0; i<DEFAULT_MAX_SIZE; i++)
                            {
                                TraceW("Data %d:Type %d, PWM %d,\n", i, TypeTable[i].MotorHP, TypeTable[i].wPwmPercent);
                            }
                            
                            TraceW("CRT Calibrate Save OK\n");
                            
                            Midware_BCS_Initial_Data();                                //初始化清零
                            by_CalFinishFlag = 1;                               //校验完成,标志置1
                        }
                    }
                }
                else
                {
                    by_StableCounter = 0;
                    
                    w_CalPWM = Midware_BCS_Calibrate_Tune(TypeTable[by_CurrentIndex].wCurrent, wCurrentActual);
                    Hal_BCS_Set_PWM_Percent(w_CalPWM);
                }
            }
        }
    }
}

UINT16 Midware_BCS_Calibrate_Tune(UINT16 Target, UINT16 Actual)
{
    float fOffset = (float)Target - (float)Actual;
    
    if(fOffset < -3 || fOffset > 3)
    {
        f_CalOutput += 1 * fOffset;
    }
    
    //不能输出小于零的值
    if(f_CalOutput < 0) f_CalOutput = 0;
    if(f_CalOutput > 2400) f_CalOutput = 2400;

    return (UINT16)f_CalOutput;
}

UCHAR Midware_BCS_CalibrationCurrent_IsOK(void)
{
    return by_CalFinishFlag;
}
UINT16 Midware_BCS_Get_MaxMotorHP(void)
{
    return MaxMotorHP;
}
UINT16 Midware_BCS_Get_MinMotorHP(void)
{
    return MinMotorHP;
}
void Midware_BCS_Init_MotorHP(void)
{
#ifdef ANALOG_DPID_MCB
  
  Hal_BCS_Set_PWM_Percent(1000-950);
  
#else
  
  UCHAR  by_Loop = 0;
  UCHAR  by_CounterTimes = 5;                                                 //总的循环次数不超过5次
  MCB_TYPE MCBType = Midware_BCS_Cal_Type();
  
  do
  {
    by_Loop = 0;
    if(by_CounterTimes) by_CounterTimes--;
    for(UCHAR i = 0; i < 5; i++)
    {
      if(MCBType != Midware_BCS_Cal_Type())
      {
        by_Loop = 1;
        MCBType = Midware_BCS_Cal_Type();
        break;
      }
    }
  }while(by_Loop && by_CounterTimes);
  MCB_Type=MCBType;
  for(UCHAR i=0;i<6;i++)
  {
    if(MCB_Type==aby_McbTypeTable[i].MCBType)      
    {
      Motor_Hp=aby_McbTypeTable[i].DefaultMotorHP;
      MaxMotorHP=aby_McbTypeTable[i].MaxMotorHP;
      MinMotorHP=aby_McbTypeTable[i].MinMotorHP;
      break;
    }
  }
  
#ifdef ANALOG_APID_MCB
  
  for(UCHAR i = 0; i < DEFAULT_MAX_SIZE; i++)
  {
    if(Motor_Hp == TypeTable[i].MotorHP)
    {
      Hal_BCS_Set_PWM_Percent(TypeTable[i].wPwmPercent);
      TraceW("BCS Type:%d, PWM:%d,\n", TypeTable[i].MotorHP, TypeTable[i].wPwmPercent);
    }
  }
  
#else
  for(UCHAR i = 0; i < DEFAULT_MAX_SIZE; i++)
  {
    if(Motor_Hp == TypeTable[i].MotorHP)
    {
      Hal_BCS_Set_PWM_Percent(TypeTable[i].wPwmPercent);
      TraceW("BCS Type:%d, PWM:%d,\n", TypeTable[i].MotorHP, TypeTable[i].wPwmPercent);
    }
  }
  
#endif
  
  
#endif
}

MCB_TYPE Midware_BCS_Cal_Type(void)
{
    MCB_TYPE MCB_Type = DEFAULT_MCB_TYPE;
    
    UINT16 Adc = Hal_BCS_ReadAdc();
    float fTemp = (500.0 / 4095.0);
    fTemp = Adc * fTemp;
    UINT16 wVoltage = (UINT16)fTemp;
    
    
    TraceW("BCS Vol:%d,\n", wVoltage);
    
    by_Error = 1;
    for(UCHAR i=0; i<DEFAULT_MAX_SIZE; i++)
    {
      if(wVoltage < aby_McbTypeTable[i].wVotageMax&& wVoltage > aby_McbTypeTable[i].wVotageMin)
      {
        MCB_Type=aby_McbTypeTable[i].MCBType;
        by_Error = 0;
      }
    }
    
    return MCB_Type;
}
MCB_TYPE BCS_Get_MCB_Type(void)
{   
  return  MCB_Type;
}
void Midware_BCS_Set_OverCurrentLimited(MOTOR_HP MotorHp)
{
  for(UCHAR i = 0; i < DEFAULT_MAX_SIZE; i++)
  {
    if(MotorHp == TypeTable[i].MotorHP)
    {
      Hal_BCS_Set_PWM_Percent(TypeTable[i].wPwmPercent);
    }
  }  
}
void Midware_BCS_1ms_Int(void)
{
    by_Flag1ms = 1;
}

MOTOR_HP Midware_BCS_Get_MotorHP(void)
{
    return Motor_Hp;
}

UCHAR Midware_BCS_Get_Error(void)
{
    return by_Error;
}

void Midware_BCS_ERP(void)
{
    Hal_BCS_ERP();
}







