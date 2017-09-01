#include "project.h"
#include "Midware_Motor_01.h"
#include "Midware_RPM_MCB_01.h"
#include "APID.h"
#include "Midware_Temp_01.h"
#include "Hal_Motor_01.h"
#include "Midware_PID_01.h"
#include "Midware_Safekey_01.h"
#include "Midware_BCS.h"


#define DELAY_OPEN_CAP_TIME	        1000                //unit 1ms,开机后先等1秒再打开副继电器
#define CAP_TIME	                3000                //unit 1ms,主继电器打开时间与副继电器的间隔
#define CLOSE_CAP_TIME	            200                 //unit 1ms,打开主继电器后关闭副继电器的时间
#define DELAY_DETECT_MOS	        500                 //unit 1ms,0.5s主继电器打开后延迟探测MOS SD时间
#define NORPM_TIME_RUN_SLOW         1000                //unit 1ms,在运行慢速阶段,无RPM检测时间
#define NORPM_TIME_RUN_FAST         500                 //unit 1ms,在运行快速阶段,无RPM检测时间
#define DEFAULT_LOWSPEED_TIME       5000                //unit 1ms,维持掉速超过规定时间
#define NORPM_TIME_START	        5000                //unit 1ms,在开始阶段,无RPM检测时间
#define NORPM_TIME_NORM	            3000                //unit 1ms,在开始阶段,无RPM检测时间


#ifdef DIGITAL_MCB

#define DEFAULT_FASTSPEED_TIME      200                 //unit 1ms,持续超过固定时间
#define DEFAULT_RPM_5MILE           1785                //5mile
#define DEFAULT_RPM_4MILE           1426                //4mile时的RPM 
#define DEFAULT_RPM_2MILE           718                 //2mile时的RPM 
#define DEFAULT_RPM_1MILE           349                 //1mile
#define DEFAULT_RPM_HALF_MILE       165                 //0.5mile
#define DEFAULT_NO_RPM_OUTPUT       80                  //0.5mile
#define DEFAULT_SPEED_PROTECT_MIN   85                  //0.5mile

#else

#define DEFAULT_FASTSPEED_TIME      500                 //unit 1ms,持续超过固定时间
#define DEFAULT_RPM_5MILE           940                 //5mile
#define DEFAULT_RPM_4MILE           752                 //4mile时的RPM 
#define DEFAULT_RPM_2MILE           376                 //2mile时的RPM 
#define DEFAULT_RPM_1MILE           186                 //1mile
#define DEFAULT_RPM_HALF_MILE       91                  //0.5mile
#define DEFAULT_NO_RPM_OUTPUT       80                  //
#define DEFAULT_SPEED_PROTECT_MIN   60                  //0.3mile

#endif


typedef enum
{
    MOTOR_OPEN_CHARGE   = 0,
    MOTOR_OPEN_MAIN,
    MOTOR_CLOSE_CHARGE,
    MOTOR_PROCESS,
    MOTOR_PROTECT,
    MOTOR_PROTECT_RESTORE,
} MOTOR_STATE;


//static UINT16 w_FastSpeedCounter = 0;
static UINT16 w_SlowSpeedCounter = 0;
static UINT16 w_NoRpmCounter = 0;
static UINT16 w_MOSCheckCounter = 0;
static UINT16 w_DelayDetectMos = DELAY_DETECT_MOS;
static UINT16 w_OCCounter = 0;
static UINT16 w_MaxActualRpm = 0;
static UINT16 w_CommonTimer; 

__no_init static UINT16 w_Rpm_Target;
__no_init static UINT16 w_Rpm_Actual;
__no_init static UINT16 w_Rpm_Roller;
__no_init static UINT16 w_Rpm_Output;
__no_init static UINT16 w_OCDisappear;
__no_init static UCHAR  bAutoCalibrate;                 //模拟下控自动校验标志
__no_init static MOTOR_ERROR MotorError;
__no_init static UCHAR TargetStable;
__no_init static MOTOR_STATE MotorState;
__no_init static TEMP_ERROR TempState;
__no_init static struct
{
    unsigned MotorSwitch        : 1;                    //马达总开关
    unsigned OverCurrent        : 1;                    //过流标志
    unsigned FlagMs             : 1;                    //1ms标志
} Motor;


static void Midware_Motor_Close(MOTOR_STATE Motor_State);
static void Midware_Motor_Protect_Speed(void);
static void Midware_Motor_Protect_NoRPM(void);
static void Midware_Motor_Protect_MOS(void);
static void Midware_Motor_Protect_Current(void);


void Midware_Motor_Initial_HW(void)
{
    Hal_Motor_Initial();
    Midware_BCS_Initial_HW();
}

void Midware_Motor_Initial_Data(void)
{
    Midware_RPM_Initial_Data();
    Midware_BCS_Initial_Data();
#ifdef DIGITAL_MCB
    Midware_PID_Initial_Data();
    TargetStable = Midware_PID_Get_TargetStable();
#elif defined ANALOG_APID_MCB
    APID_Initial_Data();
    TargetStable = APID_Get_TargetStable();
#else
    DPID_Initial_Data();
    TargetStable = DPID_Get_TargetStable();
#endif
    memset(&Motor, 0, sizeof(Motor));
    memset(&MotorError, 0, sizeof(MotorError));
    memset(&TempState, 0, sizeof(TempState));
    w_Rpm_Target = 0;
    w_Rpm_Actual = 0;
    w_Rpm_Roller = 0;
    w_Rpm_Output = 0;
    w_CommonTimer = 0;
    w_MaxActualRpm = 0;
    bAutoCalibrate = FALSE;
    Motor.MotorSwitch = 1;
    MotorState = MOTOR_OPEN_CHARGE;
    
//    w_FastSpeedCounter = 0;
    w_SlowSpeedCounter = 0;
    w_NoRpmCounter = 0;
    w_MOSCheckCounter = 0;
    w_DelayDetectMos = DELAY_DETECT_MOS;
    w_OCCounter = 0;
    w_OCDisappear = 0;
}

void Midware_Motor_Close(MOTOR_STATE Motor_State)
{
    Hal_Motor_Initial();
    Hal_Motor_Set_Relay_Main(0);                                                 //关闭主继电器
    Hal_Motor_Set_Realy_Charge(0);
    Hal_Motor_Set_PWM(0);
    w_Rpm_Actual = 0;
    w_Rpm_Target = 0;
    w_Rpm_Output = 0;
    w_CommonTimer = 0;
    w_MaxActualRpm = 0;
    w_DelayDetectMos = DELAY_DETECT_MOS;
    
#ifdef DIGITAL_MCB
    TargetStable = Midware_PID_Get_TargetStable();
    Midware_PID_Initial_Data();
#elif defined ANALOG_APID_MCB
    TargetStable = APID_Get_TargetStable();
    APID_Initial_Data();
#else
    TargetStable = DPID_Get_TargetStable();
    DPID_Initial_Data();
#endif
    
    MotorState = Motor_State;
    if(MotorState == MOTOR_PROTECT) 
    {
        Motor.MotorSwitch = 0;                                                  //跟马达相关的错误导致的关闭马达总开关
    }
}

void Midware_Motor_SetAutoCalibrate(void)
{
#ifdef DIGITAL_MCB
    
#elif defined ANALOG_APID_MCB
    bAutoCalibrate = TRUE;
    APID_SetAutoCalibrate();
#else
    bAutoCalibrate = TRUE;
    DPID_SetAutoCalibrate();
#endif
}

UCHAR Midware_Motor_GetAutoCalibrate(void)
{
#ifdef DIGITAL_MCB
    return 0;
#elif defined ANALOG_APID_MCB
    return APID_Calibrating();
#else
    return DPID_Calibrating();
#endif
}

void Midware_Motor_Process(UINT16 wTarget, UCHAR Safekey)
{
#ifdef DIGITAL_MCB
    w_Rpm_Actual = Midware_PID_Get_RPM();
    w_Rpm_Roller = w_Rpm_Actual;
#elif defined ANALOG_APID_MCB
    w_Rpm_Actual = APID_Get_RPM();
    w_Rpm_Roller = w_Rpm_Actual;
#else
    w_Rpm_Actual = DPID_Get_RPM();
    w_Rpm_Roller = w_Rpm_Actual;
#endif
    
    //安全开关处理
    if((Safekey && MotorState != MOTOR_PROTECT_RESTORE) || MotorError.Motor_HP==1)                          //安全开关移除,关闭主副继电器,关闭总开关
    {
        Midware_Motor_Close(MOTOR_PROTECT_RESTORE);
    }
    
    
    switch(MotorState)
    {
    case MOTOR_OPEN_CHARGE:
        if(Motor.MotorSwitch && w_CommonTimer >= DELAY_OPEN_CAP_TIME)           //1秒后开始打开副继电器
        {
            Hal_Motor_Set_Realy_Charge(1);
            w_CommonTimer = 0;
            MotorState = MOTOR_OPEN_MAIN;
        }
        break;
    case MOTOR_OPEN_MAIN:
        if(w_CommonTimer >= CAP_TIME)                                           //3秒后允许打开主继电器
        {
            if((bAutoCalibrate || wTarget > 0) && w_Rpm_Actual == 0)
            {
//          w_FastSpeedCounter = 0;
                w_SlowSpeedCounter = 0;
                w_NoRpmCounter = 0;
                w_MOSCheckCounter = 0;
                w_DelayDetectMos = DELAY_DETECT_MOS;
                w_OCCounter = 0;
                w_OCDisappear = 0;
                w_CommonTimer = 0;
                Hal_Motor_Set_Relay_Main(1);
                MotorState = MOTOR_CLOSE_CHARGE;
            }
        }
        break;
    case MOTOR_CLOSE_CHARGE:
        if(w_CommonTimer >= CLOSE_CAP_TIME)                                     //打开主继电器后,关闭副继电器
        {
            Hal_Motor_Set_Realy_Charge(0);
            MotorState = MOTOR_PROCESS;
        }
        break;
    case MOTOR_PROTECT:                                                         //Wait Reset
        break;
    case MOTOR_PROTECT_RESTORE:
        Midware_Temp_Get_Error(&TempState);                                             //稳度过高保护后能够自动恢复
        if(!TempState.TempMcb && !Safekey && w_Rpm_Actual == 0 && Motor.MotorSwitch) //要等待RPM归零后才能开始打开流程,防止CPU复位后立即再次驱动马达
        {
            MotorState = MOTOR_OPEN_CHARGE;
        }
        break;
    case MOTOR_PROCESS:
#ifdef DIGITAL_MCB
        if(Midware_PID_Process(wTarget, &w_Rpm_Output))
        {
            TargetStable = Midware_PID_Get_TargetStable();
            w_Rpm_Target = Midware_PID_Get_Current_Target();
            Hal_Motor_Set_PWM(w_Rpm_Output);
        }
#elif defined ANALOG_APID_MCB
        if(APID_Process(wTarget, &w_Rpm_Output))
        {
            TargetStable = APID_Get_TargetStable();
            w_Rpm_Target = APID_Get_Current_Target();
            Hal_Motor_Set_PWM(w_Rpm_Output);
        }
#else
        if(DPID_Process(wTarget, &w_Rpm_Output))
        {
            TargetStable = DPID_Get_TargetStable();
            w_Rpm_Target = DPID_Get_Current_Target();
            Hal_Motor_Set_PWM(w_Rpm_Output);
        }
#endif
        if(Motor.FlagMs)                                                        //每1ms处理一次保护功能
        {
            Motor.FlagMs = 0;
            
            if(w_Rpm_Target > 0) 
            {
                if(w_Rpm_Actual > w_MaxActualRpm)
                {
                    w_MaxActualRpm = w_Rpm_Actual;
                }
            }
            else
            {
                w_MaxActualRpm = 0;
            }
            
            Midware_Temp_Get_Error(&TempState);
#ifdef DIGITAL_MCB
            if(TempState.TempMcb)
            {
                Midware_Motor_Close(MOTOR_PROTECT_RESTORE);
            }
            
            Midware_Motor_Protect_Speed();
            Midware_Motor_Protect_NoRPM();
            Midware_Motor_Protect_MOS();
            Midware_Motor_Protect_Current();
#elif defined ANALOG_APID_MCB
            if(TempState.TempMcb)
            {
                Midware_Motor_Close(MOTOR_PROTECT_RESTORE);
            }
            
            if(!APID_Calibrating())
            {
                Midware_Motor_Protect_Speed();
                Midware_Motor_Protect_NoRPM();
            }
            
            Midware_Motor_Protect_MOS();
            Midware_Motor_Protect_Current();
#else
//            if(!DPID_Calibrating())
//            {
//                Midware_Motor_Protect_Speed();
//                Midware_Motor_Protect_NoRPM();
//            }
//            
//            Midware_Motor_Protect_MOS();
//            Midware_Motor_Protect_Current();
#endif
        }
        break;
    }
}

void Midware_Motor_Init_MotorHP(void)
{
    Midware_BCS_Init_MotorHP();
#ifdef DIGITAL_MCB
    Midware_PID_Set_MotorHP(Midware_BCS_Get_MotorHP());
#endif
}

UINT16 Midware_Motor_Get_Output(void)
{
    return w_Rpm_Output;
}

UINT16 Midware_Motor_Get_RPM(void)
{
    return w_Rpm_Actual > 0 ? w_Rpm_Actual : (w_Rpm_Output > 0 ? 1 : 0);
}

UINT16 Midware_Motor_Get_RPM_Roller(void)
{
    return w_Rpm_Roller;
}

//protect speed, 放入1ms中断处理
void Midware_Motor_Protect_Speed(void)
{
//    /*******************************过速保护***********************************/
//    
//    if(w_Rpm_Target >= DEFAULT_SPEED_PROTECT_MIN && TargetStable)
//    {
//        if(w_Rpm_Actual > (w_Rpm_Target + DEFAULT_RPM_2MILE))                   //实际值大于目标值2Mile则为过速
//        {
//            if(++w_FastSpeedCounter >= DEFAULT_FASTSPEED_TIME)                  //持续超过固定时间 ms
//            {
//                w_FastSpeedCounter = 0;
//                //MotorError.Motor_FastSpeed = 1;
//                //Midware_Motor_Close(MOTOR_PROTECT);
//            }
//        }
//        else
//        {
//            w_FastSpeedCounter = 0;
//        }
//    }
//    else
//    {
//        w_FastSpeedCounter = 0;
//    }
//    
    
    /*******************************掉速保护************************************/
    
    if(!MotorError.Motor_SlowSpeed && w_Rpm_Target >= DEFAULT_RPM_HALF_MILE && TargetStable)
    {
        UINT16 w_RPM_Step = 0;
        
        if(w_Rpm_Target <= DEFAULT_RPM_2MILE)
        {
            w_RPM_Step = DEFAULT_RPM_HALF_MILE;                                 //range is half mile when target speed <=2mile
        }
        else if(w_Rpm_Target <= DEFAULT_RPM_5MILE)
        {
            w_RPM_Step = DEFAULT_RPM_1MILE;                                     //range is 1 mile when target speed <=5mile
        }
        else
        {
            w_RPM_Step = (UINT16)((w_Rpm_Target * 3) / 10);                     //range is 30% mile when target speed >5mile
        }
        
        if((w_Rpm_Actual < (w_Rpm_Target - w_RPM_Step)) || w_Rpm_Actual == 0)
        {
            if(++w_SlowSpeedCounter >= DEFAULT_LOWSPEED_TIME)                   //维持掉速超过规定时间
            {
                w_SlowSpeedCounter = 0;
                MotorError.Motor_SlowSpeed = 1;                                 //只报错,不保护
            }
        }
        else
        {
            w_SlowSpeedCounter = 0;
        }
    }
    else
    {
        w_SlowSpeedCounter = 0;
    }
    
    
    if(w_Rpm_Target == 0)
    {
        MotorError.Motor_SlowSpeed = 0;                                         //如果目标速度为零则清除掉速告警
    }
}

//NoRPM protect, 放入1ms中断处理
void Midware_Motor_Protect_NoRPM(void)
{
#ifdef DIGITAL_MCB
    if(w_MaxActualRpm < DEFAULT_SPEED_PROTECT_MIN)
    {
        if(!Hal_Motor_Is_OverCurrent() && w_Rpm_Output >= DEFAULT_NO_RPM_OUTPUT && w_Rpm_Actual == 0)     //有输出目标值, 但没有RPM反馈时.
        {
            if(++w_NoRpmCounter >= NORPM_TIME_START)                            //超过规定时间内没有RPM, 则停止
            {
                w_NoRpmCounter = 0;
                MotorError.Motor_NoRpm = 1;
                Midware_Motor_Close(MOTOR_PROTECT);
            }
        }
        else
        {
            w_NoRpmCounter = 0;
        }
    }
    else
    {
        if(!Hal_Motor_Is_OverCurrent() && w_Rpm_Target >= DEFAULT_SPEED_PROTECT_MIN && w_Rpm_Actual == 0)                                              //有输出目标值, 但没有RPM反馈时.
        {
            UINT16 wNoRpmTimeMax = NORPM_TIME_NORM;                             //unit 1ms 无RPM检测时间
            
            if(w_Rpm_Target <= DEFAULT_RPM_1MILE)
            {
                wNoRpmTimeMax = NORPM_TIME_NORM;                                //unit 1ms,在开始阶段,无RPM检测时间
            }
            else if(w_Rpm_Target <= DEFAULT_RPM_4MILE)
            {
                wNoRpmTimeMax = NORPM_TIME_RUN_SLOW;                            //unit 1ms,在运行慢速阶段,无RPM检测时间
            }
            else
            {
                wNoRpmTimeMax = NORPM_TIME_RUN_FAST;                            //unit 1ms,在运行快速阶段,无RPM检测时间
            }
            
            if(++w_NoRpmCounter >= wNoRpmTimeMax)                               //超过规定时间内没有RPM, 则停止
            {
                w_NoRpmCounter = 0;
                wNoRpmTimeMax = NORPM_TIME_NORM;
                MotorError.Motor_NoRpm = 1;
                Midware_Motor_Close(MOTOR_PROTECT);
            }
        }
        else
        {
            w_NoRpmCounter = 0;
        }
    }
#else
    if(w_MaxActualRpm < DEFAULT_SPEED_PROTECT_MIN)
    {
        if(!Hal_Motor_Is_OverCurrent() && w_Rpm_Output >= DEFAULT_NO_RPM_OUTPUT && !Midware_RPM_IsRunning())     //有输出目标值, 但没有RPM反馈时.
        {
            if(++w_NoRpmCounter >= NORPM_TIME_START)                            //超过规定时间内没有RPM, 则停止
            {
                w_NoRpmCounter = 0;
                MotorError.Motor_NoRpm = 1;
                Midware_Motor_Close(MOTOR_PROTECT);
            }
        }
        else
        {
            w_NoRpmCounter = 0;
        }
    }
    else
    {
        if(!Hal_Motor_Is_OverCurrent() && w_Rpm_Target >= DEFAULT_SPEED_PROTECT_MIN && !Midware_RPM_IsRunning())                                              //有输出目标值, 但没有RPM反馈时.
        {
            UINT16 wNoRpmTimeMax = NORPM_TIME_NORM;                             //unit 1ms 无RPM检测时间
            
            if(w_Rpm_Target <= DEFAULT_RPM_1MILE)
            {
                wNoRpmTimeMax = NORPM_TIME_NORM;                                //unit 1ms,在开始阶段,无RPM检测时间
            }
            else if(w_Rpm_Target <= DEFAULT_RPM_4MILE)
            {
                wNoRpmTimeMax = NORPM_TIME_RUN_SLOW;                            //unit 1ms,在运行慢速阶段,无RPM检测时间
            }
            else
            {
                wNoRpmTimeMax = NORPM_TIME_RUN_FAST;                            //unit 1ms,在运行快速阶段,无RPM检测时间
            }
            
            if(++w_NoRpmCounter >= wNoRpmTimeMax)                               //超过规定时间内没有RPM, 则停止
            {
                w_NoRpmCounter = 0;
                wNoRpmTimeMax = NORPM_TIME_NORM;
                MotorError.Motor_NoRpm = 1;
                Midware_Motor_Close(MOTOR_PROTECT);
            }
        }
        else
        {
            w_NoRpmCounter = 0;
        }
    }
#endif
}

//MOS 暴冲保护, 放入1ms中断处理
void Midware_Motor_Protect_MOS(void)
{
    if(w_DelayDetectMos == 0)                                                   //打开继电器后延迟0.5秒时间,再开始检测MOS SD信号
    {
        if(Hal_Motor_Is_MOSD())
        {
            if(++w_MOSCheckCounter >= 20)                                       //暴冲信号持续时间 20ms
            {
                w_MOSCheckCounter = 0;
                w_DelayDetectMos = DELAY_DETECT_MOS;
                MotorError.Motor_MOS = 1;
                Midware_Motor_Close(MOTOR_PROTECT);
            }
        }
        else
        {
            w_MOSCheckCounter = 0;
        }
    }
    else
    {
        w_DelayDetectMos--;
    }
}

//过流端口检测, 放入1ms中断处理
void Midware_Motor_Protect_Current(void)
{
    if(Hal_Motor_Is_OverCurrent())
    {
        if(!Motor.OverCurrent)                                                  //第一次过流，清零计数器
        {
            w_OCCounter = 0;
            w_OCDisappear = 0;                                                  //过流消失重新计数
            Motor.OverCurrent = 1;
        }
        
        if(w_OCCounter < 0xFFFF) ++w_OCCounter;
        if(w_OCCounter >= 4000)                                                 //单次过流4秒或共过流超过5次2s,则报错
        {
            MotorError.Motor_OverCurrent = 1;
            Midware_Motor_Close(MOTOR_PROTECT);
        }
    }
    else
    {
        if(Motor.OverCurrent)                                                   //如果上一状态是过流
        {
            if(++w_OCDisappear >= 100)                                          //如果过流消失超过0.1s
            {
                w_OCCounter = 0;
                w_OCDisappear = 0;
                Motor.OverCurrent = 0;
            }
        }
        else
        {
            w_OCCounter = 0;
            w_OCDisappear = 0;
        }
    }
}

void Midware_Motor_1ms_Int(void)
{
    Motor.FlagMs = 1;
#ifdef DIGITAL_MCB
    Midware_RPM_1ms_Int();
    Midware_PID_1ms_Int();
#elif defined ANALOG_APID_MCB
    Midware_RPM_1ms_Int();
    APID_1ms_Int();
#else
    Midware_RPM_1ms_Int();
    DPID_1ms_Int();
#endif
    
    if(w_CommonTimer < 0xFFFF) w_CommonTimer++;
}

void Midware_Motor_Get_Error(MOTOR_ERROR *stError)
{
    memcpy(stError, &MotorError, sizeof(MotorError));
}

UCHAR Midware_Motor_IsRunning(void)
{
    return (w_Rpm_Actual && w_Rpm_Output);
}

UCHAR Midware_Motor_RPM_IsRunning(void)
{
    return w_Rpm_Actual > 0;
}

UCHAR Midware_Motor_GetIsInUsed(void)
{
    return Midware_PID_GetIsInUsed();
}

UCHAR Midware_Motor_GetStep(void)
{
    return Midware_PID_GetStep();
}
void Midware_Motor_Set_MotorHP(UINT16 MotorHp)
{
  UINT16 Motorhp=0;
  if(MotorHp < Midware_BCS_Get_MinMotorHP() || MotorHp>Midware_BCS_Get_MaxMotorHP())
  {
    MotorError.Motor_HP=1;
  }
  else
  {
    if(MotorHp%25==0)
    {
      
      if(BCS_Get_MCB_Type()==GLOBAL_150_200HP_220V 
       ||BCS_Get_MCB_Type()==GLOBAL_200_250HP_220V
       ||BCS_Get_MCB_Type()==GLOBAL_200_325HP_220V)
      {
        MotorError.Motor_HP=0;
        Motorhp=MOTOR_150_220+(MotorHp-150)/25;
      }
      else
      {
        MotorError.Motor_HP=0;
        Motorhp=MOTOR_150_110+(MotorHp-150)/25;
      }
      Midware_BCS_Set_OverCurrentLimited((MOTOR_HP)Motorhp);
      
      #ifdef DIGITAL_MCB
          Midware_PID_Set_MotorHP((MOTOR_HP)Motorhp);                           
      #endif      
    }
    else
    {
      MotorError.Motor_HP=1;
    }   
  }

}
void Midware_Motor_ERP(void)
{
    Midware_Motor_Close(MOTOR_PROTECT);                                           //关主/副继电器  //副继电器关闭，再进入ERP
    Midware_BCS_ERP();
    Midware_RPM_ERP();
    Hal_Motor_ERP();
}



