#include "Hal_Motor_01.h"
#include "gpio.h"
#include "ftm.h"
#include "sim.h"


//////////////////////////////////////////////////////////////////////
//                       副主继电器工作流程
//             开机先打开副继电器,start后再打开主继电器
//////////////////////////////////////////////////////////////////////

#ifdef DIGITAL_MCB

#define PWM_PORT                GPIOB
#define PWM_PIN                   GPIO_PTH2

#define MOTOR_PWM_MAX	        2475	                //最大值限制 95%值限制  here
#define MOTOR_PWM_FRE	        2500	                //16K

#elif defined ANALOG_APID_MCB

#define PWM_PORT                GPIOB
#define PWM_PIN                 GPIO_PTH2
#define MOTOR_PWM_MAX	        5555	                //最大值限制 无限制
#define MOTOR_PWM_FRE	        5555                    //225Hz Clk 40M/32

#else

#define PWM_PORT                GPIOA
#define PWM_PIN                 GPIO_PTB5
#define MOTOR_PWM_MAX	        2400	                //最大值限制 95%值限制
#define MOTOR_PWM_FRE	        2500	                //16K

#endif



#define RELAY_CHARGE_PORT       GPIOA
#define RELAY_CHARGE_PIN        GPIO_PTD2

#define RELAY_MAIN_PORT         GPIOA
#define RELAY_MAIN_PIN          GPIO_PTD3

#define MOS_SD_PORT             GPIOA
#define MOS_SD_PIN              GPIO_PTD0

#define OVER_CURRENT_PORT       GPIOA
#define OVER_CURRENT_PIN        GPIO_PTD1


static UCHAR by_PwmInited = 0;                                                  //用于飞思卡尔IC,未初始化就写PWM值的话会当机

static UCHAR Hal_Motor_Get_IO_Filter(GPIO_Type *Port, GPIO_PinType Pin, UINT16 Times)
{
    UCHAR  by_Loop = 0;
    UCHAR  by_CounterTimes = 5;                                                 //总的循环次数不超过5次
    UINT32 w_Out = GPIO_Read(Port) & (1 << Pin % 32);
    
    do
    {
        by_Loop = 0;
        if(by_CounterTimes) by_CounterTimes--;
        for(UCHAR i = 0; i < Times; i++)
        {
            if(w_Out != GPIO_Read(Port) & (1 << Pin % 32))
            {
                by_Loop = 1;
                w_Out = GPIO_Read(Port) & (1 << Pin % 32);
                break;
            }
        }
    }while(by_Loop && by_CounterTimes);
    
    return (w_Out ? 1 : 0);
}

void Hal_Motor_Initial(void)
{
    by_PwmInited = 0;
    GPIO_Init(PWM_PORT,  1 << (PWM_PIN % 32), GPIO_PinOutput);
	 GPIO_Init(RELAY_MAIN_PORT, 1 << RELAY_MAIN_PIN, GPIO_PinOutput);
	 GPIO_Init(RELAY_CHARGE_PORT, 1 << RELAY_CHARGE_PIN, GPIO_PinOutput);
    GPIO_Init(MOS_SD_PORT, 1 << MOS_SD_PIN, GPIO_PinInput);
    GPIO_Init(OVER_CURRENT_PORT, 1 << OVER_CURRENT_PIN, GPIO_PinInput);

    Hal_Motor_Set_Relay_Main(0);
    Hal_Motor_Set_Realy_Charge(0);
    Hal_Motor_PWM_Initial();
    by_PwmInited = 1;
}

void Hal_Motor_PWM_Initial(void)
{
#ifdef DIGITAL_MCB
    
    SIM_RemapFTM1CH0ToPTH2();
    SIM->SCGC |= SIM_SCGC_FTM1_MASK;
    FTM1->CONTROLS[0].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    FTM_SetModValue(FTM1, MOTOR_PWM_FRE);
    FTM1->CONTROLS[0].CnV = 0;
    FTM_ClockSet(FTM1, FTM_CLOCK_SYSTEMCLOCK, FTM_CLOCK_PS_DIV1);
    Hal_Motor_Set_PWM(0);
    
#elif defined ANALOG_APID_MCB
    
    SIM_RemapFTM1CH0ToPTH2();
    SIM->SCGC |= SIM_SCGC_FTM1_MASK;
    FTM1->CONTROLS[0].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    FTM_SetModValue(FTM1, MOTOR_PWM_FRE);
    FTM1->CONTROLS[0].CnV = 0;
    FTM_ClockSet(FTM1, FTM_CLOCK_SYSTEMCLOCK, FTM_CLOCK_PS_DIV32);
    Hal_Motor_Set_PWM(0);
    
#else
    
    SIM_RemapFTM2CH5ToPTB5();
    SIM->SCGC |= SIM_SCGC_FTM2_MASK;
    FTM2->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    FTM_SetFTMEnhanced(FTM2);
    FTM_SetModValue(FTM2, MOTOR_PWM_FRE);
    FTM2->CONTROLS[5].CnV = 0;
    FTM_ClockSet(FTM2, FTM_CLOCK_SYSTEMCLOCK, FTM_CLOCK_PS_DIV1);
    Hal_Motor_Set_PWM(0);
    
#endif
}

void Hal_Motor_Set_PWM(UINT16 w_Dat)
{
    if(!by_PwmInited)
    {
        by_PwmInited = 1;
        Hal_Motor_PWM_Initial();
    }
    
    if(w_Dat >= MOTOR_PWM_MAX)
    {
        w_Dat = MOTOR_PWM_MAX;
    }
    
#ifdef DIGITAL_MCB
    FTM_SetChannelValue(FTM1, FTM_CHANNEL_CHANNEL0, w_Dat);
#elif defined ANALOG_APID_MCB
    FTM_SetChannelValue(FTM1, FTM_CHANNEL_CHANNEL0, w_Dat);
#else
    FTM_SetChannelValue(FTM2, FTM_CHANNEL_CHANNEL5, w_Dat);
#endif
}

void Hal_Motor_Set_Realy_Charge(UCHAR by_Enable)
{
    if(by_Enable == 0)
    {
	GPIO_PinClear(RELAY_CHARGE_PIN);
    }
    else
    {
		GPIO_PinSet(RELAY_CHARGE_PIN);
    }
}

void Hal_Motor_Set_Relay_Main(UCHAR by_Enable)
{
    if(by_Enable == 0)
    {
        GPIO_PinClear(RELAY_MAIN_PIN);
    }
    else
    {
        GPIO_PinSet(RELAY_MAIN_PIN);
    }
}

// 1 = OverCurrent, 0 = NoOverCurrent
UCHAR Hal_Motor_Is_OverCurrent(void)
{
#ifdef DIGITAL_MCB
    return Hal_Motor_Get_IO_Filter(OVER_CURRENT_PORT, OVER_CURRENT_PIN, 5);
#elif defined ANALOG_APID_MCB
    return !Hal_Motor_Get_IO_Filter(OVER_CURRENT_PORT, OVER_CURRENT_PIN, 5);
#else
    return !Hal_Motor_Get_IO_Filter(OVER_CURRENT_PORT, OVER_CURRENT_PIN, 5);
#endif
    
}

// 1 = 保护时, 0 = 未保护时
UCHAR Hal_Motor_Is_MOSD(void)
{
    return Hal_Motor_Get_IO_Filter(MOS_SD_PORT, MOS_SD_PIN, 5);
}

void Hal_Motor_ERP(void)
{
    by_PwmInited = 0;
    FTM_DeInit(FTM2);
    GPIO_Init(PWM_PORT, 1 << (PWM_PIN % 32), GPIO_PinInput);                                         // PWM Pin
    GPIO_Init(RELAY_MAIN_PORT, 1 << RELAY_MAIN_PIN, GPIO_PinInput);                           // RELAY_Main
    GPIO_Init(RELAY_CHARGE_PORT, 1 << RELAY_CHARGE_PIN, GPIO_PinInput);                       // Relay_Charge
    GPIO_Init(MOS_SD_PORT, 1 << MOS_SD_PIN, GPIO_PinInput);                                   // OverCurrent
    GPIO_Init(OVER_CURRENT_PORT, 1 << OVER_CURRENT_PIN, GPIO_PinInput);                       // MOS_NG
}

