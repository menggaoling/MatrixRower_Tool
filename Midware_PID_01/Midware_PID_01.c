#include "Midware_PID_01.h"
#include "Midware_Trace_01.h"
#include "Midware_RPM_MCB_01.h"
#include "Midware_BCS.h"

//��С RPM 170 * 80 = 13600 / 60 = 226.6/s ��С����ʱ��Ϊ 1000ms / 226.6 = 4.4ms
//ÿһ��countʱ��Ϊ  1,000,000(1s) / (40,000,000 / 32) = 0.8us
#define TIME_BASE_OSP           937500              //60000000(ʱ��60��) / (64)            //0.8us * 80 = 64
#define DEFAULT_DECRE_MIN       165                 //1.5mile 510
#define DEFAULT_DECRE_MID_MIN   450                 //1.5mile 510
#define DEFAULT_DECRE_MID       1360                //4mile 1360
#define DEFAULT_DECRE_MAX       3500                //10.0mile 3500
#define DEFAULT_OUTPUT_PWM_MAX  4950                //���RPM���ֵ ���PWMΪ2500*2 99%

#define DEFAULT_STEP_START      1
#define DEFAULT_STEP_MID        3
#define DEFAULT_STEP_ADD        5                   //1000/20 = 50, 50*5 = 250, 4300/250 = 17.2��, ��0�����ٵ�4300�����Ҫʱ��
#define DEFAULT_STEP_DEC        7                   //Ĭ�����Step  1000/20 = 50*7 = 350RPM/s�ļ��ٶ�, 4300/350 = 12.2��.��4300���ٵ�0�����Ҫʱ��

#define DEFAULT_TIME_MS         10                  //���ε���ʱ�� ms
#define DEFAULT_RPM_NO_KP       50                  //RPMĿ��ֵС�����ֵʱ,KPΪ0
#define DEFAULT_MILE_03         85                  //<=>0.3mil/h��������ϵ��ٶ��ж�ֵ
#define DEFAULT_MILE_05         165                 //<=>0.5mil/h��������ϵ��ٶ��ж�ֵ
#define DEFAULT_MILE_1          349                 //1mile
#define DEFAULT_MILE_2          800                 //xmile
#define DEFAULT_STABLE_SPEED    349                 //1mile
#define DEFAULT_TABLE_NUM       10
#define DEFAULT_STEP_BUFF_SIZE  20


typedef enum
{
    PID_IDLE        = 0,
    PID_RUN,
    PID_STOP,
} PID_STATE;


/***********************************PID Table**********************************/
typedef struct
{
    UINT16  Target;
    UINT16  OffsetMinP;
    UINT16  OffsetMaxP;
    UINT16  OffsetMinN;
    UINT16  OffsetMaxN;
    UINT16  StepMaxAdd;         //���ٶ�
    UINT16  StepMaxDec;         //���ٶȲ���������ÿ�β������������
} OFFSET_TABLE;

typedef struct
{
    MOTOR_HP MotorHp;
    float Kp;
    float KpMin;
    float KpMid;
    float Ki;
    float Kd;
    float OffsetMinKp;
    float OffsetMidKp;
    float OffsetMinKi;
    float OffsetMidKi;
    float OffsetMinKd;
    float OffsetMidKd;
    UINT16 StartPWM;
    UINT16 StartPWM_Limit;
    UINT16 MinPWM;
} PID_TABLE;

static const PID_TABLE PIDTable[NUM_OF_MOTOR] = 
{
  
    {MOTOR_150_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},
    {MOTOR_175_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},   //OK 1.5
    {MOTOR_200_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},   //
    {MOTOR_225_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},   //
    {MOTOR_250_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},   //OK 12.21
    {MOTOR_275_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},
    {MOTOR_300_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},
    {MOTOR_325_220,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 150},
    {MOTOR_150_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_175_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_200_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_225_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_250_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_275_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},
    {MOTOR_300_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},   //OK
    {MOTOR_325_110,   5.50, 3.5, 2.5, 0.08, 0.20, 0.05, 0.15, 0.005, 0.010, 0.002, 0.005, 100, 1400, 200},   //OK
};

//������ʾʵ��ֵ����Ŀ��ֵ
static const OFFSET_TABLE kpOffsetRef[DEFAULT_TABLE_NUM] = 
{
    {3900, 45, 100, 55,  180, 160, 160}, //
    {3500, 35, 75,  45,  100, 150, 150}, //
    {3000, 25, 40,  40,  60,  140, 140}, //
    {2300, 20, 35,  35,  70,  130, 130}, //
    {1850, 20, 30,  35,  60,  100, 100}, //
    {1450, 20, 30,  20,  55,  90,  90 }, //
    {1000, 15, 30,  10,  45,  90,  90 }, //
    {700,  10, 20,  10,  45,  80,  80 }, //
    {350,  10, 20,  10,  35,  75,  70 }, //
    {0,    5,  10,  10,  35,  75,  45 }, //
};

/************************************Wave Cal**********************************/
typedef struct
{
    UCHAR  Running;
    UCHAR  RunningCounter;
    UCHAR  Restore;
    UINT32 StepTotal;
} STEP_TYPE;
/************************************Wave Cal**********************************/

__no_init static UCHAR  by_RunStep;                 //
__no_init static UINT16 w_ActualRPM;                //ʵ��RPM
__no_init static UINT16 w_LastActualRPM;            //��һ��ʵ��RPM
__no_init static UINT16 w_TargetCurrent;            //��ǰĿ��ֵ
__no_init static UINT16 w_Output;                   //���ֵ
__no_init static UINT16 w_TargetStableCounter;      //Ŀ��ֵ�ȶ���ʱ
__no_init static UINT32 w_OSP_Buff[BUFFER_SIZE];   //OSP����Buff
__no_init static float fKpRef;                      //����ʵ��Ŀ��RPMֵ����KPֵ
__no_init static float fOffsetRecord;               //
__no_init static PID_STATE PidState;                //PID����״̬
__no_init static PID_TABLE PidTableInfor;           //
__no_init static OFFSET_TABLE OffsetRefInfor;       //
__no_init static STEP_TYPE StepCalc;                //
__no_init static MOTOR_HP Motor_HP;
__no_init static struct
{
    float Offset;                                   //����ƫ��ֵ
    float OffsetLast;                               //������һ��ƫ��ֵ
    float OffsetPre;                                //��������һ��ƫ��ֵ
    float Kp, Ki, Kd;                               //������������֡�΢��ϵ��
    float Output;                                   //�������ֵ
    float OutputPre;
} PID;

__no_init static struct
{
    unsigned FlagCalc       : 1;                    //�ɼ����־
    unsigned TargetStable   : 1;                    //Ŀ��ֵ�Ƿ��ȶ�
    unsigned UpdateData     : 1;                    //Ŀ��ֵ����ʱ����ز���Ҳ���ŵ���
    unsigned NoRpmProctect  : 1;                    //
    unsigned NoRpmCheck     : 1;                    //
    unsigned NormalRun      : 1;
    unsigned BuffFlag       : 1;
    UCHAR IgnoreCounter;
} b;


static UINT16 Midware_PID_Tune(UINT16 Target, UINT16 Actual);
static UINT16 Midware_PID_Calc_Target(UINT16 Target, UINT16 Actual);
static UINT16 Midware_PID_Cal_RPM(void);
static float  Midware_PID_Calc_Pid_Value(UINT16 Target, UINT16 Actual);
static void   Midware_PID_Initial_OSP(void);
static UINT32 Midware_PID_Filter_OSP(UINT32 *pData32);
static void   Midware_PID_StepCal(UINT16 Actual);
static UINT16 Midware_PID_Filter_RPM(UINT16 wNowRpm);


void Midware_PID_Initial_Data(void)
{
    Midware_PID_Initial_OSP();
    memset(&PID, 0, sizeof(PID));
    memset(&b, 0, sizeof(b));
    memset(&PidTableInfor, 0, sizeof(PidTableInfor));
    memset(&OffsetRefInfor, 0, sizeof(OffsetRefInfor));
    PidState = PID_IDLE;
    w_TargetCurrent = 0;
    w_Output = 0;
    fOffsetRecord = 0.0;
    by_RunStep = 0;
    w_TargetStableCounter = 0;
    Midware_PID_Set_MotorHP(Motor_HP);
    memset(&StepCalc, 0, sizeof(StepCalc));
}

void Midware_PID_Set_MotorHP(MOTOR_HP MotorHp)
{
    UCHAR by_Flag = 0;
    
    if(MotorHp < NUM_OF_MOTOR)
    {
        Motor_HP = MotorHp;
        for(UCHAR i = 0; i < NUM_OF_MOTOR; i++)
        {
            if(PIDTable[i].MotorHp == Motor_HP)
            {
                memcpy(&PidTableInfor, &PIDTable[i], sizeof(PID_TABLE));
                by_Flag = 1;
            }
        }
    }
    
    if(!by_Flag)
    {
        memcpy(&PidTableInfor, &PIDTable[MOTOR_325_220], sizeof(PID_TABLE));
    }
    
    PID.Output = PidTableInfor.StartPWM;
    fKpRef = PidTableInfor.Kp;
}

//�̶���ʱ�����һ��
UCHAR Midware_PID_Process(UINT16 wTargetRPM, UINT16 *pw_Out)
{
    if(b.FlagCalc)
    {
        b.FlagCalc = FALSE;
        w_ActualRPM = Midware_PID_Cal_RPM();                                            //���㵱ǰʵ��RPM
        
        switch(PidState)
        {
        default:
        case PID_STOP:                                                          //ֹͣʱ����ֹͣ
            if(w_Output >= 2)
            {
                w_Output -= 2;
            }
            else
            {
                w_Output = 0;
                PidState = PID_IDLE;
            }
            break;
        case PID_IDLE:
            if(wTargetRPM > 0) 
            {
                Midware_PID_Initial_Data();
                PidState = PID_RUN;
                by_RunStep = 1;
            }
            break;
        case PID_RUN:
            {
                switch(by_RunStep)
                {
                default:
                case 1:
                    if(wTargetRPM >= 30)
                    {
                        by_RunStep = 2;
                    }
                    break;
                case 2:
                    if(wTargetRPM < 30)
                    {
                        by_RunStep = 3;
                    }
                    break;
                case 3:
                    wTargetRPM = 0;
                    break;
                }
                
                
                UINT16 w_Target = Midware_PID_Calc_Target(wTargetRPM, w_ActualRPM);     //Ŀ��ֵ����,�Ӽ��ٶ�ʱ,Ŀ��ֵһ��һ��ƽ�
                if(w_Target == 0)
                {
                    by_RunStep = 0;
                    w_TargetCurrent = 0;
                    PidState = PID_STOP;
                }
                else
                {
                    if(!b.NoRpmProctect)
                    {
                        w_Output = Midware_PID_Tune(w_Target, w_ActualRPM);
                        Midware_PID_StepCal(w_ActualRPM);
                    }
                }
            }
            break;
        }
        
        if(w_Output > DEFAULT_OUTPUT_PWM_MAX) w_Output = DEFAULT_OUTPUT_PWM_MAX;
        *pw_Out = w_Output >> 1;
        Trace("A:%d, B:%d,\n", w_ActualRPM, w_Output);
        return 1;
    }
    else
    {
        return 0;
    }
}
INT32 IncValue;
UINT16 Midware_PID_Tune(UINT16 Target, UINT16 Actual)
{
   
//    if(Actual == 0 && Target >= DEFAULT_MILE_03 && PID.Output > PidTableInfor.StartPWM_Limit)
//    {
//       Trace("target:%d,%d,\n", Target, PID.Output);;
//    }    
//    else 
    {
          PID.Offset = (float)Target - (float)Actual;
         if((PID.Offset < -2 || PID.Offset > 2))
         {
              //KPֵ����
              PID.Kp = Midware_PID_Calc_Pid_Value(Target, Actual);
              
              //���㹫ʽһ: ������
              float Inc = (PID.Kp * (PID.Offset - PID.OffsetLast)) + \
                   (PID.Ki * (PID.Offset)) + \
                        (PID.Kd * (PID.Offset - 2 * PID.OffsetLast + PID.OffsetPre));
              
              if(b.NormalRun && b.TargetStable && Target < DEFAULT_DECRE_MID)
              {
                   if(PID.Offset > fOffsetRecord)
                   {
                        if(!b.BuffFlag)
                        {
                             b.BuffFlag = 1;
                             b.IgnoreCounter = 0;
                             PID.OutputPre = PID.Output;
                        }
                   }
                   else
                   {
                        if(b.BuffFlag && PID.Offset <= 0)
                        {
                             if(PID.Output > PID.OutputPre)
                             {
                                  PID.Output = PID.OutputPre;
                                  b.IgnoreCounter = 5;
                             }
                             else
                             {
                                  PID.OutputPre = 0;
                                  b.IgnoreCounter = 0;
                             }
                             
                             b.BuffFlag = 0;
                        }
                   }
              }
              else
              {
                   b.BuffFlag = 0;
                   b.IgnoreCounter = 0;
                   PID.OutputPre = 0;
              }
              
              //ƫ��ֵ����
              PID.OffsetPre = PID.OffsetLast;
              PID.OffsetLast = PID.Offset;
              if(b.IgnoreCounter) 
              {
                   b.IgnoreCounter-- ;
              }
              else
              {
                   PID.Output += Inc;
              }
         }
         else
         {
              PID.OffsetPre = 0;
              PID.OffsetLast = 0;
         }
         
         //�������С�����ֵ
         if(PID.Output < 0) PID.Output = 0;
         if(PID.Output > DEFAULT_OUTPUT_PWM_MAX) PID.Output = DEFAULT_OUTPUT_PWM_MAX;
         
         //��Сֵ����
         if(Target >= DEFAULT_MILE_03 && PID.Output < PidTableInfor.MinPWM)
         {
              PID.Output = PidTableInfor.MinPWM;                                      //��������С��������
         }
         
         //���ر���,����������������PWM����,��ֹû��RPM�ź�ʱ,�ᱩ��
         if(Actual == 0 && Target <= DEFAULT_MILE_03 && PID.Output > PidTableInfor.StartPWM_Limit)
         {
              PID.Output = PidTableInfor.StartPWM_Limit;
         }
    }
    
    return (UINT16)PID.Output;
}

float Midware_PID_Calc_Pid_Value(UINT16 Target, UINT16 Actual)
{
    float fKpOut = fKpRef;
    
    if(Target < DEFAULT_MILE_03)                                                //������ֹͣ�����ٴ���
    {
        fKpOut = 0.03;
        PID.Ki = 0.03;
        PID.Kd = 0;
    }
    else
    {
        //offset��Χ�ڴ���
        float fOffset = (float)Target - (float)Actual;
        PID.Ki = PidTableInfor.Ki;
        PID.Kd = PidTableInfor.Kd;
        
        if(fOffset < OffsetRefInfor.OffsetMinP && fOffset > -OffsetRefInfor.OffsetMinN)
        {
            if(Target < DEFAULT_DECRE_MID_MIN)
            {
                fKpOut /= 2;
                PID.Ki = 0.08;
                PID.Kd = 0.2;
            }
            else
            {
                fKpOut = PidTableInfor.OffsetMinKp;
                PID.Ki = PidTableInfor.OffsetMinKi;
                PID.Kd = PidTableInfor.OffsetMinKd;
            }
            
            b.NormalRun = 1;
        }
        else if(fOffset < OffsetRefInfor.OffsetMaxP && fOffset > -OffsetRefInfor.OffsetMaxN)
        {
            if(Target < DEFAULT_DECRE_MID_MIN)
            {
                fKpOut /= 2;
                PID.Ki = 0.08;
                PID.Kd = 0.2;
            }
            else
            {
                fKpOut = PidTableInfor.OffsetMidKp;
                PID.Ki = PidTableInfor.OffsetMidKi;
                PID.Kd = PidTableInfor.OffsetMidKd;
            }
            
            b.NormalRun = 1;
        }
        else
        {
            if(Target < DEFAULT_DECRE_MID_MIN)
            {
                PID.Kd = PidTableInfor.Kd;
                
                if(fOffset < -DEFAULT_MILE_1)                                   //����Ŀ���ٶ�ָ��ֵ,�����,ʹ�ٶȿ��ٽ�����
                {
                    PID.Ki = PidTableInfor.Ki*2;
                }
                else if(fOffset > fOffsetRecord)
                {
                    if(b.NormalRun && b.TargetStable)
                    {
                        PID.Ki = PidTableInfor.Ki * 10;
                    }
                }
                else
                {
                    PID.Ki = PidTableInfor.Ki / 2.0;
                }
            }
            else if(Target < DEFAULT_DECRE_MAX)
            {
                PID.Kd = PidTableInfor.Kd / 2.0;
                
                if(fOffset < -OffsetRefInfor.OffsetMinN)
                {
                    PID.Ki = PidTableInfor.Ki;
                }
                else if(fOffset > fOffsetRecord)
                {
                    if(b.NormalRun && b.TargetStable)
                    {
                        PID.Ki = PidTableInfor.Ki * 10;
                    }
                }
                else
                {
                    PID.Ki = PidTableInfor.Ki / 2.0;
                }
            }
            else
            {
                PID.Kd = 0.0;
            }
        }
    }
    
    if(fKpOut < 0) fKpOut = 0.0;                                                //Kpֵ���Ʋ���Ϊ����
    if(fKpOut > fKpRef) fKpOut = fKpRef;                                        //Kpֵ���Ʋ��ܳ������ֵ
    if(PID.Ki < 0) PID.Ki = 0;
    if(PID.Kd < 0) PID.Kd = 0;
    
    return fKpOut;
}

//�Ӽ��ٶ�ʱ,Ŀ��ֵһ��һ��ƽ�
UINT16 Midware_PID_Calc_Target(UINT16 Target, UINT16 Actual)
{
    static UCHAR by_TimeTick = 0;
    
    if(++by_TimeTick >= 2)//20ms
    {
        by_TimeTick = 0;
        
        UINT16 wTarget = Target;
        UINT16 wOut = wTarget;
        UCHAR  by_Step = 0;
        
        if(wTarget == w_TargetCurrent)
        {
            if(!b.TargetStable)
            {
                if(++w_TargetStableCounter > 50)                                //1s
                {
                    w_TargetStableCounter = 0;
                    b.TargetStable = 1;
                }
            }
            
            wOut = w_TargetCurrent;
            b.UpdateData = 1;
        }
        else 
        {
            b.NormalRun = 0;
            
            if(wTarget > w_TargetCurrent)                                       //����Ŀ���ٶ�
            {
                if(w_TargetCurrent < DEFAULT_MILE_05)
                {
                    by_Step = 1;
                }
                else if(w_TargetCurrent < DEFAULT_MILE_1)
                {
                    by_Step = 3;
                }
                else
                {
                    by_Step = DEFAULT_STEP_ADD;
                }
                
                if(wTarget > (w_TargetCurrent + by_Step))
                {
                    if((w_TargetCurrent >= DEFAULT_MILE_03 && Actual < DEFAULT_MILE_03 - 15) ||     //�ȴ��������������Ŀ��ֵ
                       (w_TargetCurrent >= DEFAULT_RPM_NO_KP && w_TargetCurrent < DEFAULT_MILE_03 && Actual < w_TargetCurrent - 15)) //�ȴ�RPM��Ŀ��ֵ����ʱ������Ŀ��ֵ
                    {
                        wOut = w_TargetCurrent;
                    }
                    else
                    {
                        wOut = w_TargetCurrent + by_Step;
                    }
                }
                else
                {
                    wOut = wTarget;
                }
                
                w_TargetStableCounter = 0;
                b.TargetStable = 0;
                b.UpdateData = 0;
            }
            else                                                                //��СĿ���ٶ�
            {
                if(w_TargetCurrent < DEFAULT_MILE_03)
                {
                    by_Step = 2;
                }
                else if(w_TargetCurrent < DEFAULT_MILE_1)
                {
                    by_Step = 3;
                }
                else
                {
                    by_Step = DEFAULT_STEP_DEC;
                }
                
                if(w_TargetCurrent - wTarget > by_Step)
                {
                    wOut = w_TargetCurrent - by_Step;
                }
                else
                {
                    wOut = wTarget;
                }
                
                w_TargetStableCounter = 0;
                b.TargetStable = 0;
                b.UpdateData = 0;
            }
            
            
            fOffsetRecord = (float)wOut * 0.3;
            if(fOffsetRecord < DEFAULT_MILE_03) fOffsetRecord = DEFAULT_MILE_03;
            
            
            for(UCHAR i = 0; i < DEFAULT_TABLE_NUM; i++)
            {
                if(wOut >= kpOffsetRef[i].Target)
                {
                    memcpy(&OffsetRefInfor, &kpOffsetRef[i], sizeof(OFFSET_TABLE));
                    break;
                }
            }
            
            float fKpPercent = 0.0;
            if(wOut >= DEFAULT_DECRE_MID)                                       //Kp˥������
            {
                if(wOut >= DEFAULT_DECRE_MAX)                                   //RPM����7DEFAULT_DECRE_MIN��,ʵ��RPMԽ��,KPֵ��ԽС.
                {
                    fKpPercent = 0.3;                                           //0.3
                }
                else
                {
                    fKpPercent = wOut - DEFAULT_DECRE_MID;
                    fKpPercent = fKpPercent * 0.7;                              //0.7
                    fKpPercent = fKpPercent / (float)(DEFAULT_DECRE_MAX - DEFAULT_DECRE_MID);
                    fKpPercent = 1.0 - fKpPercent;
                }
                
                fKpRef = PidTableInfor.KpMid * fKpPercent;
            }
            else if(wOut >= DEFAULT_DECRE_MID_MIN)
            {
                fKpPercent = wOut - DEFAULT_DECRE_MID_MIN;
                fKpPercent = fKpPercent / (float)(DEFAULT_DECRE_MID - DEFAULT_DECRE_MID_MIN);
                fKpPercent = 1.0 - fKpPercent;
                
                fKpRef = PidTableInfor.KpMid + (PidTableInfor.KpMin - PidTableInfor.KpMid) * fKpPercent;
            }
            else if(wOut >= DEFAULT_DECRE_MIN)
            {
                fKpPercent = wOut - DEFAULT_DECRE_MIN;
                fKpPercent = fKpPercent / (float)(DEFAULT_DECRE_MID_MIN - DEFAULT_DECRE_MIN);
                fKpPercent = 1.0 - fKpPercent;
                
                fKpRef = PidTableInfor.KpMin + (PidTableInfor.Kp - PidTableInfor.KpMin) * fKpPercent;
            }
            else
            {
                fKpRef = PidTableInfor.Kp;
            }
        }
        
        w_TargetCurrent = wOut;
    }
    
    if(Target > 0 && w_TargetCurrent == 0) w_TargetCurrent = 1;
    
    return w_TargetCurrent;
}

UCHAR Midware_PID_Get_TargetStable(void)
{
    return (UCHAR)b.TargetStable;
}

UINT16 Midware_PID_Get_Current_Target(void)
{
    return w_TargetCurrent;
}

void Midware_PID_1ms_Int(void)
{
    static UINT16 w_TickMs_Counter = 0;
    if(++w_TickMs_Counter >= DEFAULT_TIME_MS)//10ms
    {
        w_TickMs_Counter = 0;
        b.FlagCalc = TRUE;
    }
}


/***********************************OSP****************************************/
void Midware_PID_Initial_OSP(void)
{
    w_ActualRPM = 0;
    w_LastActualRPM = 0;
    memset(w_OSP_Buff, 0, sizeof(w_OSP_Buff));
}

UINT16 Midware_PID_Get_RPM_Roller(void)
{
    return w_ActualRPM;
}

UINT16 Midware_PID_Get_RPM(void)
{
    return w_ActualRPM;
}

UINT16 Midware_PID_Cal_RPM(void)
{
    static UCHAR by_LastRPMCounter = 0;
    UINT16 wOut = 0;
    
    //RPM����
    if(Midware_RPM_Get_PulseData(w_OSP_Buff))
    {
        UINT32 wOSP = Midware_PID_Filter_OSP(w_OSP_Buff);
        
        //OSP����Ϊ0,��RPMҲΪ��
        if(wOSP > 0)
        {
            //rpm = 60,000,000(΢��) / ��ǰ���� * һ������ʱ�� * һ��RPMΪ80���ж��ź�
            wOut = (UINT16)(TIME_BASE_OSP / wOSP);
        }
        
        if(b.NoRpmProctect || b.NoRpmCheck)
        {
            if(++by_LastRPMCounter >= 30)
            {
                by_LastRPMCounter = 0;
                if(b.NoRpmProctect)
                {
                    TraceW("Release:%d\n", wOut);
                    TraceW("PID Release Protect\n");
                    TraceW("\n");
                }
                
                b.NoRpmProctect = 0;
                b.NoRpmCheck = 0;
            }
        }
        else
        {
            by_LastRPMCounter = 0;
        }
        
        wOut = Midware_PID_Filter_RPM(wOut);
    }
    else
    {
        by_LastRPMCounter = 0;
        
        //�Ƿ���NoRPM����
        if(w_TargetCurrent >= DEFAULT_MILE_2)
        {
            if(!b.NoRpmProctect && !b.NoRpmCheck)
            {
                b.NoRpmCheck = 1;
                b.NoRpmProctect = 1;
                TraceW("PID Protect Lost Speed\n");
                TraceW("Target Current:%d\n", w_TargetCurrent);
            }
        }
        else if(w_TargetCurrent > DEFAULT_MILE_03)
        {
          
            if(!b.NoRpmProctect && !b.NoRpmCheck)
            {
                b.NoRpmCheck = 1;
                
                UCHAR  by_MaxRef = BUFFER_SIZE;
                
                if(w_TargetCurrent >= 3000)
                {
                  by_MaxRef = BUFFER_SIZE;
                }
                else if(w_TargetCurrent >= 2750)
                {
                  by_MaxRef = 15;
                }
                else if(w_TargetCurrent >= 2000)
                {
                  by_MaxRef = 10;
                }
                else if(w_TargetCurrent >= 1350)
                {
                  by_MaxRef = 9;
                }
                else if(w_TargetCurrent >= 850)
                {
                  by_MaxRef = 7;
                }
                else if(w_TargetCurrent >= 500)
                {
                  by_MaxRef = 6;
                }
                else
                {
                  by_MaxRef = 5;
                }                
                
                
                
                UINT16 wRpm[BUFFER_SIZE] = {0};
                
                 //ת��ΪRPMֵ
                for(UCHAR i=0; i<BUFFER_SIZE; i++)
                {
                    wRpm[i] = (UINT16)(TIME_BASE_OSP / w_OSP_Buff[i]);
                    TraceW("rpm1:%d,%d\n", wRpm[i],i);
                }
                //ת��ΪRPMֵ
                for(UCHAR i=0; i<by_MaxRef; i++)
                {
                    wRpm[i] = (UINT16)(TIME_BASE_OSP / w_OSP_Buff[i]);
                    TraceW("rpm:%d,%d\n", wRpm[i],i);
                }
                
                
                //����ݼ�����
                UCHAR by_DecCount = 0;
                for(UCHAR i=0; i<(by_MaxRef-1); i++)
                {
                    if(wRpm[i] < wRpm[i+1])
                    {
                        by_DecCount++;
                    }
                }
                TraceW("by_DecCount:%d,\n", by_DecCount);
                if(by_DecCount<by_MaxRef-1)
                {
                  b.NoRpmProctect = 1;
//                  for(UCHAR i=0; i<BUFFER_SIZE; i++)
//                  {
//                    TraceW("Buff:%d,\n", w_OSP_Buff[i]);
//                  }
                  TraceW("PID Protect\n");
                }
//                //ɾ�����MAX_OSP_BUFF - 10��ֵ,��ֻ��10��ֵ
//                for(UCHAR i=0; i<(BUFFER_SIZE - 10); i++)
//                {
//                    UINT16 wTempMax = wRpm[0];
//                    UCHAR by_DelIndex = 0;
//                    
//                    for(UCHAR k=1; k<BUFFER_SIZE; k++)
//                    {
//                        if(wRpm[k] > wTempMax)
//                        {
//                            wTempMax = wRpm[k];
//                            by_DelIndex = k;
//                        }
//                    }
//                    
//                    wRpm[by_DelIndex] = 0;
//                }
                
                
//                //��������
//                UINT32 wTotal = 0;
//                for(UCHAR i=0; i<BUFFER_SIZE; i++)
//                {
//                    wTotal += wRpm[i];
//                }
                
//                TraceW("by_DecCount1:%d, AvgRpm1:%d,\n", by_DecCount, wTotal / 10);
//                //�ж��Ƿ���������
//                if(by_DecCount < (BUFFER_SIZE / 2) && (wTotal / 10) > (DEFAULT_MILE_03 - 10))
//                {
//                    b.NoRpmProctect = 1;
//                    for(UCHAR i=0; i<BUFFER_SIZE; i++)
//                    {
//                        TraceW("Buff:%d,\n", w_OSP_Buff[i]);
//                    }
//                    TraceW("PID Protect\n");
//                    TraceW("by_DecCount:%d, AvgRpm:%d,\n", by_DecCount, wTotal / 10);
//                }
            }
        }
        
        Midware_PID_Initial_OSP();
    }
    
    return wOut;
}

UINT32 Midware_PID_Filter_OSP(UINT32 *pData32)
{
    //����OSP�ź� BUFFER_SIZE
    UINT32 wBuff[3] = {0};
    UCHAR  by_Index = 0;
    UCHAR  by_MaxRef = BUFFER_SIZE;
    
    if(w_TargetCurrent >= 3000)
    {
        by_MaxRef = BUFFER_SIZE;
    }
    else if(w_TargetCurrent >= 2750)
    {
        by_MaxRef = 15;
    }
    else if(w_TargetCurrent >= 2000)
    {
        by_MaxRef = 10;
    }
    else if(w_TargetCurrent >= 1350)
    {
        by_MaxRef = 9;
    }
    else if(w_TargetCurrent >= 850)
    {
        by_MaxRef = 7;
    }
    else if(w_TargetCurrent >= 500)
    {
        by_MaxRef = 6;
    }
    else
    {
        by_MaxRef = 5;
    }
    
    
    /**************************************************************/
    //ɾ�����2��ֵ,�˳�����ж¿׵�
    for(UCHAR i=0; i<2; i++)
    {
        wBuff[i] = pData32[0];
        by_Index = 0;
        
        for(UCHAR k=1; k<by_MaxRef; k++)
        {
            if(pData32[k] > wBuff[i])
            {
                wBuff[i] = pData32[k];
                by_Index = k;
            }
        }
        
        pData32[by_Index] = 0;
    }
    
    //�ҳ�3�����ֵ,���ֵ����RPM��С
    for(UCHAR i=0; i<3; i++)
    {
        wBuff[i] = pData32[0];
        by_Index = 0;
        
        for(UCHAR k=1; k<by_MaxRef; k++)
        {
            if(pData32[k] > wBuff[i])
            {
                wBuff[i] = pData32[k];
                by_Index = k;
            }
        }
        
        pData32[by_Index] = 0;
    }
    
    //3��ֵ�����ƽ��
    UINT32 wCount = 0;
    for(UCHAR i=0; i<3; i++)
    {
        wCount += wBuff[i];
    }
    return (wCount / 3);
}

void Midware_PID_StepCal(UINT16 Actual)
{
    static UINT16 w_TimeOut_Counter = 0;
    static UINT16 w_TimeInterval = 0;
    static UINT16 w_RPMBuff[DEFAULT_STEP_BUFF_SIZE] = {0};
    
    
    //����Buff
    for(UCHAR i=(DEFAULT_STEP_BUFF_SIZE-1); i>0; i--)
    {
        w_RPMBuff[i] = w_RPMBuff[i-1];
    }
    w_RPMBuff[0] = Actual;
    
    
    //����ݼ�,��������
    UCHAR by_DecCount = 0;
    UCHAR by_IncCount = 0;
    for(UCHAR i=0; i<(DEFAULT_STEP_BUFF_SIZE-1); i++)
    {
        if(w_RPMBuff[i] < w_RPMBuff[i+1])
        {
            by_DecCount++;
        }
        else if(w_RPMBuff[i] > w_RPMBuff[i+1])
        {
            by_IncCount++;
        }
    }
    
    
    if(w_TimeInterval < 0xFFFF) w_TimeInterval++;
    
    //Step ״̬����
    if(StepCalc.Restore)
    {
        if(by_IncCount > (DEFAULT_STEP_BUFF_SIZE / 2 + 1))
        {
            StepCalc.Restore = 0;
        }
    }
    else
    {
        if(by_DecCount > (DEFAULT_STEP_BUFF_SIZE / 2 + 1))
        {
            StepCalc.Restore = 1;
            
            if(!StepCalc.Running)
            {
                if(w_TimeInterval < 200)
                {
                    if(StepCalc.RunningCounter >= 5) 
                    {
                        StepCalc.Running = 1;
                        w_TimeOut_Counter = 0;
                    }
                    else
                    {
                        StepCalc.RunningCounter++;
                    }
                }
                else
                {
                    StepCalc.RunningCounter = 0;
                }
            }
            else
            {
                if(StepCalc.RunningCounter)
                {
                    StepCalc.StepTotal += StepCalc.RunningCounter;
                    StepCalc.RunningCounter = 0;
                }
                
                if(StepCalc.StepTotal < 0xFFFFFFFF) StepCalc.StepTotal++;
                
                
                if(w_TimeInterval > 200)
                {
                    if(StepCalc.RunningCounter >= 3) 
                    {
                        StepCalc.Running = 0;
                    }
                    else
                    {
                        StepCalc.RunningCounter++;
                    }
                }
                else
                {
                    StepCalc.RunningCounter = 0;
                }
                
                
                w_TimeOut_Counter = 0;
            }
            
            //Trace("Running:%d, Total:%d, Itv:%d\n", StepCalc.Running, StepCalc.StepTotal, w_TimeInterval);
            w_TimeInterval = 0;
        }
    }
    
    
    //����һ��ʱ���޶���ʱ,������״̬
    if(w_TimeOut_Counter > 500)                                                 // 500*20 = 10000 = 10s��û�ж�������Ϊ���Ѿ��߿�
    {
        w_TimeOut_Counter = 0;
        if(StepCalc.Running)
        {
            StepCalc.Running = 0;
            //Trace("Running:%d, Counter:%d,\n", StepCalc.Running, StepCalc.StepTotal);
        }
    }
    else
    {
        w_TimeOut_Counter++;
    }
}

UINT32 Midware_PID_GetStep(void)
{
    return StepCalc.StepTotal;
}

UCHAR Midware_PID_GetIsInUsed(void)
{
    return StepCalc.Running;
}

UINT16 Midware_PID_Filter_RPM(UINT16 wNowRpm)
{
    UINT16 wOut = wNowRpm;
    
    if(w_TargetCurrent >= DEFAULT_MILE_03 && w_TargetCurrent >= OffsetRefInfor.Target)
    {
        if(wNowRpm > w_LastActualRPM)                                           //���Ӳ�������
        {
            if(wNowRpm - w_LastActualRPM >= OffsetRefInfor.StepMaxAdd)
            {
                wOut = w_LastActualRPM + OffsetRefInfor.StepMaxAdd;             //ʵ��RPM���Ӳ���
            }
        }
        else
        {
            if(w_LastActualRPM - wNowRpm >= OffsetRefInfor.StepMaxDec)          //ʵ��RPM��������
            {
                wOut = w_LastActualRPM - OffsetRefInfor.StepMaxDec;
            }
        }
    }
    
    w_LastActualRPM = wOut;                                                     //������һ�ε�RPM
    return wOut;
}
