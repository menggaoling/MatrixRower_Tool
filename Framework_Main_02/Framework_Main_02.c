#include "Framework_Main_02.h"
#include "Midware_Update_01.h"
#include "Midware_Digital_01.h"
#include "Midware_Incline_01.h"
#include "Midware_Motor_01.h"
#include "Midware_ErrorCode_01.h"
#include "Framework_System_02.h"
#include "Midware_Timer_01.h"
#include "Midware_Led_01.h"
#include "Midware_Led_01.h"
#include "Midware_Safekey_01.h"
#include "Midware_Trace_01.h"
#include "Midware_Nvflash_01.h"
#include "Midware_Update_01.h"
#include "Midware_Temp_01.h"
#include "Midware_RPM_MCB_01.h"
#include "Midware_BCS.h"
#include "sim.h"


//****************************************************************************//
// �����
//1.�¿����� -- ��Ҫ�����ƶ�
//
//****************************************************************************//

#define LCB_ERP_LEAVE       0x00
#define LCB_ERP_ENTER       0xFF
#define LCB_ERP_ACK         0x01


#ifdef DIGITAL_MCB

//#define TYPE                    0x04//0xD0
#define VER_MAJOR               1
#define VER_MINOR               2
#define DCI_VERSION             "DJ1.0000A9ACD3X1.240C7E4FA8"

#elif defined ANALOG_APID_MCB

//#define TYPE                    0xB0
#define VER_MAJOR               1
#define VER_MINOR               1
#define DCI_VERSION             "DJ1.0000A9ACD3X1.240C7E4FA8"

#else

//#define TYPE                    0xB0
#define VER_MAJOR               1
#define VER_MINOR               1
#define DCI_VERSION             "DJ1.0000A9ACD3X1.240C7E4FA8"

#endif

#define BOOT_VER_MAJOR_ADR      0x3FF4
#define BOOT_VER_MINOR_ADR      0x3FF8

#define C_PASSWORD              0xa55a



typedef enum
{
    TIMER_COMMON = 0,
    TIMER_INITED,
    TIMER_SLEEP_DELAY,
    TIMER_WAKEUP_DELAY,
    TIMER_UPDATE_WAIT,
    TIMER_INC_ENABLE,
    TIMER_TEST,
} TIMER_TYPE;


typedef enum
{
    UNLOCK_NONE = 0,
    UNLOCK_INIT,
    UNLOCK_STATUS,
} UNLOCK_STEP;




extern UINT16 __checksum;
//__root const UCHAR DeviceModelType@ ".ModelType" = TYPE;
__root const UCHAR DeviceVerMajor@ ".VerMajor" = VER_MAJOR;
__root const UCHAR DeviceVerMinor@ ".VerMinor" = VER_MINOR;

__no_init static UINT16 passWord;
__no_init static struct
{
    UCHAR  by_Inited;                       //����������ʼ��,1���״̬��λ
    UCHAR  by_ErpOn;                        //����ERP��־
    UCHAR  by_LcbUpdate;                    //���±�־λ
    UCHAR  by_CalCurrent;                   //����У�������־
    UINT16 w_CurrentActual;                 //��ǰ���ʵ�ʵ���,
    UINT16 w_RpmTarget;                     //�յ���Ŀ��RPM
    UNLOCK_STEP  UnlockStep;                //�Ͽؽ����ȫ���ز���
} Main;


static void Main_Initial_HW(void);
static void Main_Initial_Data(void);
static void Main_ERP_Process(void);
//static void Main_Update_Process(void);
static void Main_Com_Process(void);
static void Main_ResetLog(void);
int main(void)
{
 if(__checksum == 0) __checksum = 1;
 
    Main_Initial_HW();
    Main_Initial_Data();
    memset(&Main, 0, sizeof(Main));
    Main_ResetLog();
    if(passWord != C_PASSWORD)
    {
        passWord = C_PASSWORD;
        while(!Midware_Timer_Counter(TIMER_COMMON, 50)) Midware_System_WDOG_Feed();               //��Դ�������ٵ�1����,��ֹ����ʱ�ظ���λ
        
    }
    Midware_Motor_Init_MotorHP();
    while(1)
    {
        Midware_System_WDOG_Feed();
       Main_Com_Process();
        Midware_Motor_Process(Main.w_RpmTarget, Midware_ErrorCode_GetStatus(SAFEKEY));
        Midware_Incline_Process(Midware_ErrorCode_GetStatus(SAFEKEY));
        Midware_ErrorCode_Process();
        //Main_Update_Process();
        Main_ERP_Process();
        Midware_Temp_Process();
        
        if(Main.by_CalCurrent)
        {
            Midware_BCS_CalibrationCurrent_Process(Main.w_CurrentActual);
        }
    }
}

void Main_Initial_HW(void)
{
   
    
    //DINT();                                                                     //�ر��ж��ܿ���  
    DisableInterrupts;
    Midware_System_Initial_HW();   
    //    HW_Empty_IO_Initial();
    Midware_Trace_Initial_HW();
    Midware_Temp_Initial_HW();
    Midware_Safekey_Initial_HW();
    Midware_Digital_Initial_HW();
    Midware_Led_Init_HW();
    Midware_RPM_Initial_HW();
    Midware_Motor_Initial_HW();
    Midware_Incline_Initial_HW();
    Midware_System_WDOG_Feed();
    
    EnableInterrupts;                                                                     //���ж��ܿ���
    
}
void Main_Initial_Data(void)
{
    Midware_Nvflash_Initial_Data();
    Midware_ErrorCode_Initial_Data();
    Midware_Trace_Initial_Data();
    Midware_Digital_Initial_Data();
    Midware_Safekey_Initial_Data();
    Midware_Motor_Initial_Data();
    Midware_Incline_Initial_Data();
    Midware_Led_Init_Data();
    Midware_Timer_Initial_Data();
    Midware_Temp_Initial_Data();
    Midware_Update_Init_Data();
    Midware_BCS_Initial_Data();
    Midware_RPM_Initial_Data();
}
/*
void Main_Update_Process(void)
{
    if(Main.by_LcbUpdate)                                                       //�յ��˸����¿�����
    {
        if(!Midware_Digital_Have_Buff() || Midware_Timer_Counter(TIMER_UPDATE_WAIT, 350)) //3.5s������������Ϊ�յĻ���λ������MCB UPDATE�׶�
        {
            Update_Write_Flag();
            Main.by_LcbUpdate = 0;
            NVIC_SystemReset();
        }
    }
}*/

void Main_ERP_Process(void)
{
    if(Main.by_ErpOn)
    {
        Main.w_RpmTarget = 0;
        
        if(Midware_Motor_Get_RPM() == 0 && Midware_Timer_Counter(TIMER_SLEEP_DELAY, 100))                               //��RPMΪ0���ٽ���ERP
        {
            Midware_System_WDOG_Feed();
            Midware_Motor_ERP();
            Midware_Incline_ERP();
            Midware_Safekey_ERP();
            Midware_Led_ERP();
            Midware_Trace_Erp();
            Midware_Digital_Erp();
//            HW_Empty_IO_Erp();
            
            Midware_Timer_Clear(TIMER_WAKEUP_DELAY);
            while(!Midware_Timer_Counter(TIMER_WAKEUP_DELAY, 30)) Midware_System_WDOG_Feed();     //�ӽ���˯�ߵ�������ʱ300ms
            
            while(1)                                                            //sleep���ڴ˵ȴ�����
            {
                Midware_System_ERP();
                stop();                                                         //cpu��ͣ����
                Midware_System_EnableRTC();
                
                Midware_Timer_Clear(TIMER_COMMON);
                while(!Midware_Digital_Get_Rx())                                  //���Ϊ�͵�ƽǰ�ҳ�������200ms�Ļ�,��λ. ��׼Ϊ500ms
                {
                    Midware_System_WDOG_Feed();
                    if(Midware_Timer_Counter(TIMER_COMMON, 20))
                    {
                      Midware_ErrorCode_Initial_Data();
                      Midware_Motor_Initial_Data();
                      Midware_Incline_Initial_Data();
                      NVIC_SystemReset();                                     //�ȴ���λ
                    }
                }
            }
        }
    }
}

void Main_Com_Process(void)
{
    DataUnion16       un16;
    MCB_STATUS McbStatus;
    COMMAND_TX CommandTx;
    COMMAND_RX CommandRx;
    
    un16.Word = 0;
    McbStatus.status = 0;
    memset(&CommandTx, 0, sizeof(CommandTx));
    memset(&CommandRx, 0, sizeof(CommandRx));
    
    
    if(Main.by_Inited && Midware_Timer_Counter(TIMER_INITED, 100)) Main.by_Inited = 0;
    if(Midware_Motor_IsRunning())
    {
        if(Midware_Timer_Counter(TIMER_INC_ENABLE, 200))                                //2s�������
        {
            Midware_Incline_Allow_Act();
        }
    }
    else
    {
        Midware_Timer_Clear(TIMER_INC_ENABLE);
    }
    
    Midware_Digital_Process(&CommandRx);
    if(CommandRx.Cmd != CMD_NONE)
    {
        switch(CommandRx.Cmd)
        {
        case CMD_UPDATE_GET_TYPE:
        case CMD_UPDATE_SET_VER:
        case CMD_UPDATE_GET_NUM:
        case CMD_UPDATE_SET_DATA:
        case CMD_UPDATE_FINISHED:
            {
                DataUnion32 backData;
                if(CommandRx.Cmd == 0x01 || CommandRx.Cmd == 0x04) CommandTx.Len = 3;
                else if(CommandRx.Cmd == 0x03) CommandTx.Len = 2;
                else if(CommandRx.Cmd == 0x05) CommandTx.Len = 1;
                
                backData = Midware_Update_Process(CommandRx.Cmd, &CommandRx.Data[0],BCS_Get_MCB_Type());
                CommandTx.Data[0] = backData.Byte[0];
                CommandTx.Data[1]= backData.Byte[1];
                CommandTx.Data[2] = backData.Byte[2];
                break;
            }    
              
          
        case CMD_START_CAL_CURRENT:                                             //��ʼУ��
            Main.by_CalCurrent = 1;
            break;
        case CMD_CAL_CURRENT:                                                   //���ʵʱ����ֵ
            un16.Byte[0] = CommandRx.Data[0];
            un16.Byte[1] = CommandRx.Data[1];
            Main.w_CurrentActual = un16.Word;
            
            CommandTx.Len = 1;
            CommandTx.Data[0] = Midware_BCS_CalibrationCurrent_IsOK();            //��ȡ״̬. �յ�����1��ʾУ�����,����ֵ����У����
            break;
        case CMD_EXIT_CAL_CURRENT:                                              //�˳�У��ģʽ
            Main.by_CalCurrent = 0;
            break;
        case CMD_AUTO_CALIBRATE:                                                //ģ���¿��Զ�У��
            Midware_Motor_SetAutoCalibrate();
            
            CommandTx.Len = 1;
            CommandTx.Data[0] = Midware_Motor_GetAutoCalibrate();                 //ģ���¿��Զ�У�鵱ǰ����
            break;
        case CMD_GET_TM_IN_USED:
            CommandTx.Len = 1;
            CommandTx.Data[0] = Midware_Motor_GetIsInUsed();
            break;
        case CMD_INITIAL:
            {
                if(!Main.by_Inited)
                {
                    Main.by_Inited = 1;
                    //McbStatus.McbInitial = 1;
                    Midware_Timer_Clear(TIMER_INITED);
                }
                
                McbStatus.MCBinitial = 0;
                Main.UnlockStep = UNLOCK_INIT;
                break;
            }
        case CMD_GET_VERSION:                                                   //��ȡ�汾��,�ɰ�ͨѶЭ�鷽ʽ,Ϊ��ֱ���ϰ�ͨѶЭ��
            {
                un16.Word = 0xFF00;                                             //�ϰ汾�Ǳ��ȡʱ����ʹ��,ͨѶЭ��涨ʹ�����ֵ,��Ҫ�޸�;
                CommandTx.Len = 2;
                CommandTx.Data[0] = un16.Byte[0];
                CommandTx.Data[1] = un16.Byte[1];
                break;
            }
        case CMD_GET_NEW_VERSION:                                               //��ȡ�汾��,�°�ͨѶЭ�鷽ʽ
            {
                if(CommandRx.Data[0] == 0xF0)
                {
                    CommandTx.Len = 6;
                    CommandTx.Data[5] = 0xF0;
                    CommandTx.Data[4] = 0x01;
                    CommandTx.Data[3] = BCS_Get_MCB_Type();
                    CommandTx.Data[2] = 0x00;
                    CommandTx.Data[1] = VER_MAJOR;
                    CommandTx.Data[0] = VER_MINOR;
                }
                break;
            }
        case CMD_GET_DCI_VERSION:                                               //��ȡDCI�汾��,̨���Ͽ���,�ظ����ݲ�����Э����Ĭ��ֵ.Ҫ��27������
            {
                CommandTx.Len = strlen(DCI_VERSION);
                strcpy((char*)CommandTx.Data, DCI_VERSION);
                break;
            }
        case CMD_UPDATE_PROGRAM:
            {
                Main.by_LcbUpdate = 0;                                          //�ȳ�ʼ��Ϊ�����£�
                CommandTx.Len = 1;
                CommandTx.Data[0] = 'N';
                
                if(CommandRx.Len == 2)
                {
                    un16.Byte[0] = CommandRx.Data[0];
                    un16.Byte[1] = CommandRx.Data[1];
                    if(un16.Byte[1] == 'J' && un16.Byte[0] == 'S')
                    {
                        Midware_Timer_Clear(TIMER_UPDATE_WAIT);
                        Main.by_LcbUpdate = 1;
                        CommandTx.Len = 1;
                        CommandTx.Data[0] = 'Y';
                    }
                }
                break;
            }
        case CMD_SET_DRIVE_MOTOR_HP:                                            //�趨�¿����HP(������С)
            {
                if(CommandRx.Len == 2)
                {
                    un16.Byte[0] = CommandRx.Data[0];
                    un16.Byte[1] = CommandRx.Data[1];
                    Midware_Motor_Set_MotorHP(un16.Word);
                }
                break;
            }
        case CMD_GET_DRIVER_TYPE:
            {
                CommandTx.Len = 1;
                CommandTx.Data[0] = 0;                                          //0x00 : with encoder 0xFF : without encoder
                break;
            }
        case CMD_SKIP_CURRENT_ERROR:
            {
                Midware_ErrorCode_Skip_Last();
                break;
            }
        case CMD_GET_ERROR_CODE:
            {
                CommandTx.Len = 2;
                un16.Word = Midware_ErrorCode_Get_ErrorCode();
                CommandTx.Data[0] = un16.Byte[0];
                CommandTx.Data[1] = un16.Byte[1];
                break;
            }
        case CMD_SET_WORK_STATUS:
            {
                un16.Byte[0] = CommandRx.Data[0];
                un16.Byte[1] = CommandRx.Data[1];
                
                switch(Main.UnlockStep)
                {
                case UNLOCK_INIT:
                    if(un16.Word == 0x00)
                    {
                        Main.UnlockStep = UNLOCK_STATUS;
                    }
                    break;
                case UNLOCK_STATUS:
                    if(un16.Word == 0x01)
                    {
                        Midware_ErrorCode_ResetSafekey();                               //�����ȫ���ش���״̬
                        Main.UnlockStep = UNLOCK_NONE;
                    }
                    break;
                default:
                    break;
                }
                break;
            }
        case CMD_SPECIAL_EXT_COMMAND:
            {
                CommandTx.Len = 1;
                CommandTx.Data[0] = 0;
                break;
            }
        case CMD_SET_MOTOR_RPM_MAX:
            {
                break;
            }
        case CMD_SET_MOTOR_RPM:
            {
                if(CommandRx.Len == 2)
                {
                    un16.Byte[0] = CommandRx.Data[0];
                    un16.Byte[1] = CommandRx.Data[1];
                    Main.w_RpmTarget = un16.Word & 0x7FFF;                      //���һλ����, ������Hybrid��̨
                }
                break;
            }
        case CMD_GET_MOTOR_RPM:
            {
                un16.Word = Midware_Motor_Get_RPM();
                CommandTx.Len = 2;
                CommandTx.Data[0] = un16.Byte[0];
                CommandTx.Data[1] = un16.Byte[1];
                break;
            }
        case CMD_GET_ROLLER_RPM:
            {
                un16.Word = Midware_Motor_Get_RPM_Roller();
                CommandTx.Len = 2;
                CommandTx.Data[0] = un16.Byte[0];
                CommandTx.Data[1] = un16.Byte[1];
                break;
            }
        case CMD_SET_INCLINE_ACTION:
            {
                Midware_Incline_Set_Action(CommandRx.Data[0]);
                break;
            }
        case CMD_SET_INCLINE_LOCATION:
            {
                if(CommandRx.Len == 2)
                {
                    un16.Byte[0] = CommandRx.Data[0];
                    un16.Byte[1] = CommandRx.Data[1];
                    Midware_Incline_Set_TargetCount(un16.Word);
                }
                break;
            }
        case CMD_GET_INCLINE_LOCATION:
            {
                CommandTx.Len = 2;
                un16.Word = Midware_Incline_Get_Count();
                CommandTx.Data[0] = un16.Byte[0];
                CommandTx.Data[1] = un16.Byte[1];
                break;
            }
        case CMD_SET_ERPMODE:
            {
                if(CommandRx.Len == 1)
                {
                    if(CommandRx.Data[0] == LCB_ERP_ENTER)                      //set MCB to sleep
                    {
                        CommandTx.Len = 1;
                        CommandTx.Data[0] = LCB_ERP_ACK;
                        Main.by_ErpOn = 1;
                        Midware_Timer_Clear(TIMER_SLEEP_DELAY);
                    }
                    else if(CommandRx.Data[0] == LCB_ERP_LEAVE)                 //set MCB to wakeup
                    {
                        CommandTx.Len = 1;
                        CommandTx.Data[0] = LCB_ERP_ACK;
                        Main.by_ErpOn = 0;
                    }
                }
                break;
            }
        case CMD_GET_OSP:
        case CMD_SET_INTERVALS:
        case CMD_SET_PWM_ADDSTEP:
        case CMD_SET_PWM_DECSTEP:
        case CMD_SET_PWM_STOPSTEP:
        case CMD_GET_STATUS:
        case CMD_TUNEEND_POINT_INCLINE:
        case CMD_TUNEEND_POINT_INCLINE2:
        case COM_SET_INCLINE_STROKE:
        case CMD_SET_COMPENSATION_VOLTAGE:
        case CMD_CALIBRATE:
            break;
        default:
            McbStatus.commandError = 1;
            break;
        }
        
        
        McbStatus.MCBerror      = Midware_ErrorCode_Have_Error() ? 1 : 0;
        McbStatus.ospRunning    = Midware_Motor_RPM_IsRunning() ? 1 : 0;
        McbStatus.mainMotor     = Midware_Motor_IsRunning() ? 1 : 0;
        McbStatus.inclineDown   = Midware_Incline_IsDown() ? 1 : 0;
        McbStatus.inclineUp     = Midware_Incline_IsUp() ? 1 : 0;
        
        CommandTx.Status = McbStatus.status;
        CommandTx.Cmd = CommandRx.Cmd;
        Midware_Digital_Send_Command(&CommandTx);
    }
    else
    {
        if(Midware_ErrorCode_GetStatus(DIGTIAL_TIMEOUT))
        {
            Main.w_RpmTarget = 0;                                               //��λͨѶʧȥ���Ӻ�,Target RPM������Ļ�,����
//            if(!Main.by_ErpOn)
//            {
//                Main.by_ErpOn = 1;                                              //��λͨѶʧȥ���Ӻ�,�Զ�����˯��ģʽ
//                Midware_Timer_Clear(TIMER_SLEEP_DELAY);
//            }
        }
    }
}

void Main_ResetLog(void)
{
    UCHAR by_Rst = 0;
    
    //get all status bits
    UINT32 wStatus = SIM_GetStatus(0xFF);
    
    //get all IDs
    UCHAR by_FamID = SIM_ReadID(ID_TYPE_FAMID);
    UCHAR by_SubFamID = SIM_ReadID(ID_TYPE_SUBFAMID);
    UCHAR by_RevID = SIM_ReadID(ID_TYPE_REVID);
    UCHAR by_PinID = SIM_ReadID(ID_TYPE_PINID);
    
    
    /***************************System Log Begin*******************************/
    
    TraceW("\n\n--System Log Begin--\n\n");
    TraceW("\nFamilly ID = 0x%x, Sub-family ID = 0x%x, Revision ID = 0x%x, Pin ID = 0x%x. \n",
           by_FamID, by_SubFamID, by_RevID, by_PinID);
    
    if((wStatus & SIM_SRSID_POR_MASK) && (wStatus & SIM_SRSID_LVD_MASK))
    {
        by_Rst = 1;
        TraceW("Power On Reset.\n");
    }
    
    if(!(wStatus & SIM_SRSID_POR_MASK) && (wStatus & SIM_SRSID_LVD_MASK))
    {
        by_Rst = 1;
        TraceW("LVD Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_WDOG_MASK)
    {
        by_Rst = 1;
        TraceW("WDOG Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_PIN_MASK)
    {
        by_Rst = 1;
        TraceW("Pin Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_LOC_MASK)
    {
        by_Rst = 1;
        TraceW("Loss of Clock Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_SACKERR_MASK)
    {
        by_Rst = 1;
        TraceW("Stop Mode Acknowledge Error Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_MDMAP_MASK)
    {
        by_Rst = 1;
        TraceW("MDM-AP System Reset Request.\n");
    }
    
    if(wStatus & SIM_SRSID_SW_MASK)
    {
        by_Rst = 1;
        TraceW("Software/SYSRESETREQ Reset.\n");
    }
    
    if(wStatus & SIM_SRSID_LOCKUP_MASK)
    {
        by_Rst = 1;
        TraceW("Core lockup Reset.\n");
    }
    
    if(by_Rst != 1)
    {
        TraceW("SWD Reset.\n");
    }
    
    TraceW("\n--System Log End--\n\n");
    
    /***************************System Log End*********************************/
    
    
    
    /***********************(***Deveice Information****************************/
    
    TraceW("\n\n--Deveice Information--\n");
    TraceW("\nJohnson Industries(Shanghai) Co., Ltd..\n");
    TraceW("Programer: Enble.\n");
    TraceW("MCB Project: Global MCB.\n");
//    TraceW("MCB Type: 0x%02x.\n", TYPE);
    TraceW("Version: V%d.%03d.\n", VER_MAJOR, VER_MINOR);
    TraceW("ERP: Support.\n");
    TraceW("\n--Deveice Information--\n\n");
    
    /***************************Deveices Information****************************/
}
